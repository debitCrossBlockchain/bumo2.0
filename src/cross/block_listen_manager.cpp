#include "block_listen_manager.h"
#include <common/general.h>
#include "cross/message_channel_manager.h"
#include <proto/cpp/overlay.pb.h>

namespace bumo {

	const static char* OP_CREATE_CHILD_CHAIN		= "CreateChildChain";



	BlockListenManager::BlockListenManager(){
	}

	BlockListenManager::~BlockListenManager(){
	}

	bool BlockListenManager::Initialize(){
		return true;
	}

	bool BlockListenManager::Exit(){

		return true;
	}
	/*
	bool BlockListenManager::CommitBlock(){
		HandleMainChainBlock();
		HandleChildChainBlock();
		return true;
	}
	*/
	bool BlockListenManager::HandleBlock(LedgerFrm::pointer closing_ledger){
		HandleMainChainBlock(closing_ledger);
		HandleChildChainBlock(closing_ledger);
		return true;
	}

	protocol::MESSAGE_CHANNEL_TYPE BlockListenManager::FilterTlog(std::string tlog_topic)
	{
		if (tlog_topic.empty())
			return protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_TYPE_NONE;

		if (0 == strcmp(tlog_topic.c_str(), OP_CREATE_CHILD_CHAIN))
			return protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN;
		else
			return protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_TYPE_NONE;
	}

	::google::protobuf::Message * BlockListenManager::GetMsgObject(protocol::MESSAGE_CHANNEL_TYPE msg_type)
	{
		if (msg_type == protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN)
			return new protocol::MessageChannelCreateChildChain();
		
		return nullptr;
	}

	const protocol::OperationLog * BlockListenManager::PickTransferTlog(TransactionFrm::pointer txFrm)
	{

		for (int i = 0; i < txFrm->instructions_.size(); i++)
		{
			//auto op_type = trans->operations(j).type();
			//find tlog
			//protocol::TransactionEnvStore &env_sto = txFrm->instructions_[i];
			const protocol::Transaction &trans = txFrm->instructions_[i].transaction_env().transaction();

			//must be CMC send trans
			if (trans.source_address() != General::CONTRACT_CMC_ADDRESS)
				continue;

			for (int j = 0; j < trans.operations_size(); j++)
			{
				if (protocol::Operation_Type_LOG == trans.operations(j).type())
				{
					const protocol::OperationLog &log = trans.operations(j).log();
					if (log.topic().size() == 0 || log.topic().size() > General::TRANSACTION_LOG_TOPIC_MAXSIZE){
						LOG_ERROR("Log's parameter topic size should be between (0,%d]", General::TRANSACTION_LOG_TOPIC_MAXSIZE);
						continue;
					}
					//special transaction
					if (FilterTlog(log.topic()) != protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_TYPE_NONE)
					{
						//transfer tlog params must be 5
						if (log.datas_size() != 5)
						{
							LOG_ERROR("tlog parames number should have 5,but now is ", log.datas_size());
							return nullptr;
						}
						int iSize = log.datas_size();
						LOG_INFO("get tlog topic:%s,args[0]:%s,args[1]:%s,args[2]:%s", log.topic().c_str(), log.datas(0).c_str(), log.datas(1).c_str(), log.datas(2).c_str());
						return &log;
					}
				}
			}
		}
		return nullptr;
	}

	void BlockListenManager::HandleMainChainBlock(LedgerFrm::pointer closing_ledger){
		//TODO: Handel child chain block, and call MessageChannel to send main chain proc //
		
		//bool bHaveEvent = false;
		for (size_t i = 0; i < closing_ledger->ProtoLedger().transaction_envs_size(); i++)
		{
			TransactionFrm::pointer tx = closing_ledger->apply_tx_frms_[i];
			//const protocol::Transaction &tran = ledger.transaction_envs(i).transaction();
			//
			const protocol::OperationLog *pTlog = PickTransferTlog(tx);
			if (nullptr != pTlog)
			{
				protocol::MessageChannel msg_channel;
				const std::string &strTlogParams = pTlog->datas(4);
				//tlog param(0)
				if (strTlogParams.size() == 0 || strTlogParams.size() > General::TRANSACTION_LOG_DATA_MAXSIZE)
				{
					LOG_ERROR("Log's parameter data size should be between (0,%d]", General::TRANSACTION_LOG_DATA_MAXSIZE);
					break;
				}
				//LOG_INFO("get tlog topic:%s,args[0]:%s", log.topic(), log.datas(j));
				Json::Value trans_json;
				if (!trans_json.fromString(strTlogParams)) {
					LOG_ERROR("Failed to parse the json content of the tlog");
					continue;
				}
				msg_channel.set_target_chain_id(atoi(pTlog->datas(3).c_str()));

				protocol::MESSAGE_CHANNEL_TYPE msg_type = FilterTlog(pTlog->topic());
				msg_channel.set_msg_type(msg_type);

				::google::protobuf::Message * pMsg = GetMsgObject(msg_type);
				std::string error_msg;
				if (!Json2Proto(trans_json, *pMsg, error_msg)) {
					LOG_ERROR("Failed to Json2Proto error_msg=%s",error_msg.c_str());
					continue;
				}
				msg_channel.set_msg_data(pMsg->SerializeAsString().c_str());

				MessageChannel::GetInstance()->MessageChannelProducer(msg_channel);

				delete pMsg;

			}
		}
		
	}

	void BlockListenManager::HandleChildChainBlock(LedgerFrm::pointer closing_ledger){
		//TODO: Handel child chain block, and call MessageChannel to send main chain proc 
		if (General::GetSelfChainId() != General::MAIN_CHAIN_ID)
		{
			//send to messagechanel
			protocol::LedgerHeader& ledger_header = closing_ledger->GetProtoHeader();
			protocol::MessageChannel msg_channel;
			//sendto main chain id=0
			msg_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
			msg_channel.set_msg_type(protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_SUBMIT_HEAD);
			msg_channel.set_msg_data(ledger_header.SerializeAsString().c_str());

			MessageChannel::GetInstance()->MessageChannelProducer(msg_channel);
			LOG_INFO("childChain build a block hash=%s,send msgchannel", utils::String::BinToHexString(ledger_header.hash()).c_str());
		}
	}
}
