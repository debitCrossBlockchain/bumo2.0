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

	protocol::MESSAGE_CHANNEL_TYPE BlockListenManager::str2msgtype(std::string trans_str)
	{
		if (trans_str.empty())
			return protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_TYPE_NONE;

		if (0 == strcmp(trans_str.c_str(), OP_CREATE_CHILD_CHAIN))
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

	const protocol::OperationLog * BlockListenManager::HaveTransferTlog_Main(TransactionFrm::pointer txFrm)
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
					if (str2msgtype(log.topic()) != protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_TYPE_NONE)
					{
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
		
		//protocol::Ledger& ledger = closing_ledger->ProtoLedger();
		//bool bHaveEvent = false;
		for (size_t i = 0; i < closing_ledger->ProtoLedger().transaction_envs_size(); i++)
		{
			TransactionFrm::pointer tx = closing_ledger->apply_tx_frms_[i];
			//const protocol::Transaction &tran = ledger.transaction_envs(i).transaction();
			//
			const protocol::OperationLog *pTlog = HaveTransferTlog_Main(tx);
			if (nullptr != pTlog)
			{
				protocol::MessageChannel msg_channel;
				const std::string &strTransDetail = pTlog->datas(0);
				//tlog param(0)
				if (strTransDetail.size() == 0 || strTransDetail.size() > General::TRANSACTION_LOG_DATA_MAXSIZE)
				{
					LOG_ERROR("Log's parameter data size should be between (0,%d]", General::TRANSACTION_LOG_DATA_MAXSIZE);
					break;
				}
				//LOG_INFO("get tlog topic:%s,args[0]:%s", log.topic(), log.datas(j));
				Json::Value trans_json;
				if (!trans_json.fromString(strTransDetail)) {
					LOG_ERROR("Failed to parse the json content of the tlog");
					continue;
				}
				if (trans_json.isMember("chain_id"))
					msg_channel.set_target_chain_id(trans_json["chain_id"].asInt64());

				protocol::MESSAGE_CHANNEL_TYPE msg_type = str2msgtype(pTlog->topic());
				msg_channel.set_msg_type(msg_type);
				::google::protobuf::Message * pMsg = GetMsgObject(msg_type);

				std::string error_msg;
				if (!Json2Proto(trans_json, *pMsg, error_msg)) {
					LOG_ERROR("Failed to Json2Proto error_msg=%s",error_msg.c_str());
					continue;
				}
				msg_channel.set_msg_data(pMsg->SerializeAsString().c_str());

				protocol::MessageChannelCreateChildChain x;
				x.ParseFromString(pMsg->SerializeAsString().c_str());

				MessageChannel::GetInstance()->MessageChannelProducer(msg_channel);

				delete pMsg;
				//°ü×°Event and Send
				//bHaveEvent = true;
				//break;
			}
		}
		
	}

	void BlockListenManager::HandleChildChainBlock(LedgerFrm::pointer closing_ledger){
		//TODO: Handel child chain block, and call MessageChannel to send main chain proc 
		if (0 != General::GetSelfChainId())
		{
			//send to messagechanel
			LOG_INFO("childChain build a block,send msgchannel");
		}
	}
}
