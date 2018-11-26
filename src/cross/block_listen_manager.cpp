#include "block_listen_manager.h"
#include <common/general.h>
#include "cross/message_channel_manager.h"
#include "cross/proposer_manager.h"
#include <proto/cpp/overlay.pb.h>
#include <common/private_key.h>

namespace bumo {

	const static char* OP_CREATE_CHILD_CHAIN = "createChildChain";

	void BlockListenManager::HandleBlock(LedgerFrm::pointer closing_ledger){
		if (General::GetSelfChainId() == General::MAIN_CHAIN_ID){
			HandleMainChainBlock(closing_ledger);
		}
		else{
			HandleChildChainBlock(closing_ledger);
		}
	}

	protocol::MESSAGE_CHANNEL_TYPE BlockListenManager::FilterTlog(std::string tlog_topic){
		if (tlog_topic.empty()){
			return protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_TYPE_NONE;
		}

		if (0 == strcmp(tlog_topic.c_str(), OP_CREATE_CHILD_CHAIN)){
			return protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN;
		}
		else{
			return protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_TYPE_NONE;
		}
	}

	std::shared_ptr<Message> BlockListenManager::GetMsgObject(protocol::MESSAGE_CHANNEL_TYPE msg_type){
		switch (msg_type){
		case protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN:
			return std::make_shared<protocol::MessageChannelCreateChildChain>();
		default:
			return nullptr;
		}
	}

	const protocol::OperationLog * BlockListenManager::PickTransferTlog(const protocol::Transaction &trans){
		//must be CMC send trans
		if (trans.source_address() != General::CONTRACT_CMC_ADDRESS){
			return nullptr;
		}
			
		for (int j = 0; j < trans.operations_size(); j++){
			if (protocol::Operation_Type_LOG != trans.operations(j).type())
				continue;
			const protocol::OperationLog &log = trans.operations(j).log();
			if (log.topic().size() == 0 || log.topic().size() > General::TRANSACTION_LOG_TOPIC_MAXSIZE){
				LOG_ERROR("Log's parameter topic size should be between (0,%d]", General::TRANSACTION_LOG_TOPIC_MAXSIZE);
				continue;
			}
			//special transaction
			if (FilterTlog(log.topic()) == protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_TYPE_NONE){
				continue;
			}
			//transfer tlog params must be 2
			if (log.datas_size() != 2){
				LOG_ERROR("tlog parames number should have 2,but now is ", log.datas_size());
				return nullptr;
			}
			LOG_INFO("get tlog topic:%s,args[0]:%s,args[1]:%s", log.topic().c_str(), log.datas(0).c_str(), log.datas(1).c_str());
			return &log;
		}

		return nullptr;
	}

	void BlockListenManager::DealProposerTrans(const protocol::Transaction &trans, int64_t &error_code, const std::string &error_desc, const std::string& hash) {
		//must be CMC send trans
		std::string private_key = Configure::Instance().ledger_configure_.validation_privatekey_;
		PrivateKey pkey(private_key);
		if (!pkey.IsValid()){
			LOG_ERROR("Private key is not valid");
			return;
		}

		std::string source_address = pkey.GetEncAddress();
		if (trans.source_address() != source_address){
			return;
		}
			
		std::string des_address = "";
		bool find_proposer = false;
		for (int j = 0; j < trans.operations_size(); j++){
			des_address.clear();
			switch (trans.operations(j).type())
			{
			case protocol::Operation_Type_PAY_COIN:{
				const protocol::OperationPayCoin &ope = trans.operations(j).pay_coin();
				des_address = ope.dest_address();
				break;
			}
			case protocol::Operation_Type_PAY_ASSET:{
				const protocol::OperationPayAsset &ope = trans.operations(j).pay_asset();
				des_address = ope.dest_address();
				break;
			}
			default:
				break;
			}
			if (des_address == General::CONTRACT_CMC_ADDRESS){
				find_proposer = true;
				break;
			}
		}

		if (!find_proposer){
			return;
		}

		ProposerManager::Instance().UpdateTransactionErrorInfo(error_code, error_desc, hash);
		return;
	}

	void BlockListenManager::DealTlog(const protocol::Transaction &trans){
		const protocol::OperationLog *tlog = PickTransferTlog(trans);
		if (nullptr == tlog){
			return;
		}
			
		protocol::MessageChannel msg_channel;
		const std::string &tlog_params = tlog->datas(1);
		//tlog param(0)
		if (tlog_params.size() == 0 || tlog_params.size() > General::TRANSACTION_LOG_DATA_MAXSIZE){
			LOG_ERROR("Log's parameter data size should be between (0,%d]", General::TRANSACTION_LOG_DATA_MAXSIZE);
			return;
		}
		//LOG_INFO("get tlog topic:%s,args[0]:%s", log.topic(), log.datas(j));
		Json::Value trans_json;
		if (!trans_json.fromString(tlog_params)) {
			LOG_ERROR("Failed to parse the json content of the tlog");
			return;
		}
		msg_channel.set_target_chain_id(atoi(tlog->datas(0).c_str()));

		protocol::MESSAGE_CHANNEL_TYPE msg_type = FilterTlog(tlog->topic());
		msg_channel.set_msg_type(msg_type);

		std::shared_ptr<Message> msg = GetMsgObject(msg_type);
		if (!msg)
			return;
		std::string error_msg;
		if (!Json2Proto(trans_json, *msg, error_msg)) {
			LOG_ERROR("Failed to Json2Proto error_msg=%s", error_msg.c_str());
			return;
		}
		msg_channel.set_msg_data(msg->SerializeAsString());

		MessageChannel::Instance().MessageChannelProducer(msg_channel);
	}


	void BlockListenManager::HandleMainChainBlock(LedgerFrm::pointer closing_ledger){
		for (int i = 0; i < closing_ledger->ProtoLedger().transaction_envs_size(); i++){
			TransactionFrm::pointer tx = closing_ledger->apply_tx_frms_[i];

			const protocol::Transaction &apply_tran = tx->GetTransactionEnv().transaction();
			int64_t code = (int64_t)tx->GetResult().code();
			std::string desc = tx->GetResult().desc();
			std::string hash = utils::String::BinToHexString(tx->GetContentHash()).c_str();

			DealProposerTrans(apply_tran, code, hash, hash);

			//deal append trans
			for (unsigned int i = 0; i < tx->instructions_.size(); i++){
				const protocol::Transaction &trans = tx->instructions_[i].transaction_env().transaction();
				DealTlog(trans);
				DealProposerTrans(trans, code, hash, hash);
			}
		}
	}

	void BlockListenManager::HandleChildChainBlock(LedgerFrm::pointer closing_ledger){
		//send to messagechanel
		protocol::LedgerHeader& ledger_header = closing_ledger->GetProtoHeader();
		protocol::MessageChannel msg_channel;
		//sendto main chain id=0
		msg_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
		msg_channel.set_msg_type(protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_SUBMIT_HEAD);
		msg_channel.set_msg_data(ledger_header.SerializeAsString());

		MessageChannel::Instance().MessageChannelProducer(msg_channel);
		LOG_INFO("childChain build a block hash=%s,send msgchannel", utils::String::BinToHexString(ledger_header.hash()).c_str());
	}
}
