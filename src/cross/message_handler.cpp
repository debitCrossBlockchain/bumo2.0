#include <common/storage.h>
#include <common/private_key.h>
#include <common/pb2json.h>
#include "message_handler.h"
#include "cross_utils.h"

namespace bumo {
	extern bool g_enable_;

	MessageHandler::MessageHandler(){
		init_ = false;
		received_create_child_ = false;
		cur_nonce_ = 0;
	}

	MessageHandler::~MessageHandler(){
	}

	bool MessageHandler::Initialize(){
		proc_methods_[protocol::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN] = std::bind(&MessageHandler::OnHandleCreateChildChain, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_CHILD_GENESES_REQUEST] = std::bind(&MessageHandler::OnHandleChildGenesesRequest, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_CHILD_GENESES_RESPONSE] = std::bind(&MessageHandler::OnHandleChildGenesesResponse, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_QUERY_HEAD] = std::bind(&MessageHandler::OnHandleQueryHead, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_QUERY_DEPOSIT] = std::bind(&MessageHandler::OnHandleQueryDeposit, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_DEPOSIT] = std::bind(&MessageHandler::OnHandleDeposit, this, std::placeholders::_1);

		MessageChannel &message_channel = MessageChannel::Instance();
		message_channel.RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN);
		message_channel.RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_CHILD_GENESES_REQUEST);
		message_channel.RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_CHILD_GENESES_RESPONSE);
		message_channel.RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_QUERY_HEAD);
		message_channel.RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_QUERY_DEPOSIT);
		message_channel.RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_DEPOSIT);

		if (!CheckForChildBlock()){
			return false;
		}

		init_ = true;

		PrivateKey private_key(Configure::Instance().ledger_configure_.validation_privatekey_);
		if (!private_key.IsValid()){
			LOG_ERROR("Private key is not valid");
			return false;
		}
		source_address_ = private_key.GetEncAddress();

		return true;
	}

	bool MessageHandler::Exit(){
		init_ = false;
		return true;
	}

	bool MessageHandler::CheckForChildBlock(){
		if (General::GetSelfChainId() <= General::MAIN_CHAIN_ID) {
			return true;
		}

		auto kvdb = Storage::Instance().account_db();
		std::string str_max_seq;
		if (kvdb->Get(General::KEY_LEDGER_SEQ, str_max_seq) > 0){
			return true;
		}

		GenesisConfigure &config = Configure::Instance().genesis_configure_;
		if (!config.validators_.empty() || !config.account_.empty() ||
			!config.slogan_.empty() || config.fees_.gas_price_ != 0 ||
			config.fees_.base_reserve_ != 0){
			LOG_ERROR("Other parameters of the child chain must be configured to be empty");
			return false;
		}

		int64_t last_request_time = 0;
		while (g_enable_){
			//Provides event drivers for the main thread
			int64_t cur_time = utils::Timestamp::HighResolution();
			for (auto item : bumo::TimerNotify::notifys_){
				item->TimerWrapper(utils::Timestamp::HighResolution());
			}

			if ((cur_time - last_request_time) >= 1 * utils::MICRO_UNITS_PER_SEC){
				//Send request create child message
				LOG_INFO("Waitting for create child (" FMT_I64 ") chain message..", General::GetSelfChainId());
				SendChildGenesesRequest();
				last_request_time = cur_time;
			}

			if (received_create_child_){
				LOG_INFO("Received create child (" FMT_I64 ") chain message..", General::GetSelfChainId());
				break;
			}

			utils::Sleep(1);
		}

		return true;
	}

	void MessageHandler::HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel){
		MessageChannelPoc proc;
		MessageChannelPocMap::iterator iter = proc_methods_.find(message_channel.msg_type());
		if (iter == proc_methods_.end()) {
			LOG_TRACE("Type(" FMT_I64 ") not found", message_channel.msg_type());
			return; 
		}

		//When the child chain is not initialized, it is only allowed to receive child geneses response messages
		if (General::GetSelfChainId() != General::MAIN_CHAIN_ID && !init_ 
			&& message_channel.msg_type() != protocol::MESSAGE_CHANNEL_CHILD_GENESES_RESPONSE){
			LOG_ERROR("Wating for message channel child response, but now:(" FMT_I64 ")", message_channel.msg_type());
			return;
		}

		iter->second(message_channel);
	}

	void MessageHandler::OnHandleCreateChildChain(const protocol::MessageChannel &message_channel){
		protocol::MessageChannelCreateChildChain create_child_chain;
		if (!create_child_chain.ParseFromString(message_channel.msg_data())){
			LOG_ERROR("Parse MessageChannelCreateChildChain error!");
			return;
		}

		CreateChildChain(create_child_chain);
		return;
	}

	void MessageHandler::OnHandleChildGenesesRequest(const protocol::MessageChannel &message_channel){
		protocol::MessageChannelChildGenesesRequest child_chain_request;
		protocol::MessageChannelCreateChildChain create_child_chain;
		protocol::MessageChannelChildGenesesResponse response;
		protocol::ERRORCODE error_code = protocol::ERRCODE_SUCCESS;
		std::string error_desc = "";
		if (!child_chain_request.ParseFromString(message_channel.msg_data())){
			error_desc = utils::String::Format("Parse MessageChannelChildGenesesRequest error!");
			error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}

		if (child_chain_request.chain_id() <= 0){
			error_desc = utils::String::Format("Parse MessageChannelChildGenesesRequest error,invalid chain id(" FMT_I64 ")", child_chain_request.chain_id());
			error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}
		do
		{
			Json::Value query_rets;
			Json::Value input_value;
			Json::Value params;
			input_value["method"] = "queryChildChainInfo";
			input_value["params"]["chain_id"] = child_chain_request.chain_id();
			if (protocol::ERRCODE_SUCCESS != bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input_value.toFastString(), query_rets)){
				error_desc = utils::String::Format("Query contract error!%s", query_rets.toFastString().c_str());
				error_code = protocol::ERRCODE_INVALID_PARAMETER;
				LOG_ERROR("%s", error_desc.c_str());
				break;
			}

			if (!query_rets.isArray()){
				error_desc = utils::String::Format("Query contract error! Json result is not array.%s", query_rets.toFastString().c_str());
				error_code = protocol::ERRCODE_INVALID_PARAMETER;
				LOG_ERROR("%s", error_desc.c_str());
				break;
			}

			Json::Value custom_result;
			custom_result.fromString(query_rets[Json::UInt(0)]["result"]["value"].asString());
			std::string error_msg;
			if (!Json2Proto(custom_result, create_child_chain, error_msg)){
				error_desc = utils::String::Format("Invalid contract result:%s", custom_result.toFastString().c_str());
				error_code = protocol::ERRCODE_INVALID_PARAMETER;
				LOG_ERROR("%s", error_desc.c_str());
				break;
			}
		} while (false);

		response.set_error_code(error_code);
		response.set_error_desc(error_desc);
		*response.mutable_create_child_chain() = create_child_chain;

		//Push message to child chain.
		protocol::MessageChannel message;
		message.set_target_chain_id(child_chain_request.chain_id());
		message.set_msg_type(protocol::MESSAGE_CHANNEL_CHILD_GENESES_RESPONSE);
		message.set_msg_data(response.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(message);
	}

	void MessageHandler::OnHandleChildGenesesResponse(const protocol::MessageChannel &message_channel){
		protocol::MessageChannelChildGenesesResponse child_chain_response;
		if (!child_chain_response.ParseFromString(message_channel.msg_data())){
			LOG_ERROR("Parse OnHandleChildGenesesResponse error!");
			return;
		}

		if (child_chain_response.error_code() != protocol::ERRCODE_SUCCESS){
			LOG_ERROR("OnHandleChildGenesesResponse's error code:%d, des:%s", 
				child_chain_response.error_code(), child_chain_response.error_desc().c_str());
			return;
		}

		CreateChildChain(child_chain_response.create_child_chain());
	}

	void MessageHandler::CreateChildChain(const protocol::MessageChannelCreateChildChain &create_child_chain){
		std::string validators;
		utils::StringList validator_list;
		GenesisConfigure &genesis_config = Configure::Instance().genesis_configure_;

		std::string account = create_child_chain.genesis_account();
		if (!PublicKey::IsAddressValid(account)){
			LOG_ERROR("Invalid address:%s", account.c_str());
			return;
		}

		if (create_child_chain.chain_id() != genesis_config.chain_id_){
			LOG_ERROR("Invalid chain id:("  FMT_I64 ")", create_child_chain.chain_id());
			return;
		}

		const protocol::FeeConfig &fee_config = create_child_chain.fee();
		if (fee_config.base_reserve() < 0 || fee_config.gas_price() < 0){
			LOG_ERROR("Invalid fee base reserve:(" FMT_I64 "), fee config:(" FMT_I64 ") ",
				fee_config.base_reserve(), fee_config.gas_price());
			return;
		}

		if (create_child_chain.reserve_validator_size() <= 0){
			LOG_ERROR("No reserve address.");
			return;
		}

		for (int32_t i = 0; i < create_child_chain.reserve_validator_size(); i++){
			std::string reserve_validator = create_child_chain.reserve_validator(i);
			if (!PublicKey::IsAddressValid(reserve_validator)){
				LOG_ERROR("Invalid reserve address:%s", reserve_validator.c_str());
				return;
			}
			validator_list.push_back(reserve_validator);
			validators = utils::String::Format("%s,", reserve_validator.c_str());
		}

		genesis_config.account_ = account;
		genesis_config.fees_.base_reserve_ = fee_config.base_reserve();
		genesis_config.fees_.gas_price_ = fee_config.gas_price();
		genesis_config.slogan_ = create_child_chain.slogan();
		genesis_config.validators_.clear();
		genesis_config.validators_.assign(validator_list.begin(), validator_list.end());

		LOG_INFO("Received create child chain message, validators:%s, account:%s, slogan:%s, chain id:(" FMT_I64 ")",
			validators.c_str(), create_child_chain.genesis_account().c_str(),
			create_child_chain.slogan().c_str(), create_child_chain.chain_id());

		received_create_child_ = true;
	}

	void MessageHandler::OnHandleQueryHead(const protocol::MessageChannel &message_channel){
		protocol::MessageChannelQueryHead head_query;
		protocol::ERRORCODE error_code = protocol::ERRCODE_SUCCESS;
		std::string error_desc = "";
		int64_t ledger_seq = 0;

		if (!head_query.ParseFromString(message_channel.msg_data())){
			error_desc = utils::String::Format("Parse MessageChannelQueryHead error!");
			error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}
		ledger_seq = head_query.ledger_seq();
		if (ledger_seq <= 0){
			error_desc = utils::String::Format("Parse MessageChannelQueryHead error,invalid ledger_seq(" FMT_I64 ")", ledger_seq);
			error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}

		LedgerFrm frm;
		if (!frm.LoadFromDb(ledger_seq)) {
			error_code = protocol::ERRCODE_NOT_EXIST;
			error_desc = utils::String::Format("Parse MessageChannelQueryHead error,no exist ledger_seq=(" FMT_I64 ")", ledger_seq);
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}
		protocol::LedgerHeader& ledger_header = frm.GetProtoHeader();
		//Push message to child chain.
		protocol::MessageChannel msg_channel;
		msg_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
		msg_channel.set_msg_type(protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_SUBMIT_HEAD);
		msg_channel.set_msg_data(ledger_header.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(msg_channel);
	}

	void MessageHandler::OnHandleQueryDeposit(const protocol::MessageChannel &message_channel){
		protocol::MessageChannelQueryDeposit query_deposit;
		if (General::GetSelfChainId()!=General::MAIN_CHAIN_ID){
			return;
		}

		if (!query_deposit.ParseFromString(message_channel.msg_data())){
			int64_t error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("Parse MessageChannelQueryDeposit error, err_code is (" FMT_I64 ")", error_code);
			return;
		}

		Json::Value params;
		Json::Value input_value;
		if (query_deposit.seq() == -1){
			params["chain_id"] = query_deposit.chain_id();
			input_value["method"] = "queryChildFreshDeposit";
		}
		else{
			params["chain_id"] = query_deposit.chain_id();
			params["seq"] = query_deposit.seq();
			input_value["method"] = "queryChildDeposit";
		}

		input_value["params"] = params;

		Json::Value result_list;
		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input_value.toFastString(), result_list);
		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		Json::Value object;
		object.fromString(result.c_str());
		if (error_code != protocol::ERRCODE_SUCCESS){
			LOG_ERROR("Failed to query child deposit .%d", error_code);
			return;
		}

		protocol::MessageChannelDeposit deposit;
		protocol::MessageChannel msg_channel;
		deposit.set_chain_id(object["chain_id"].asInt64());
		deposit.set_amount(object["amount"].asInt64());
		deposit.set_seq(object["seq"].asInt64());
		deposit.set_block_number(object["block_number"].asInt64());
		deposit.set_source_address(object["source_address"].asString());
		deposit.set_address(object["address"].asString());

		msg_channel.set_target_chain_id(query_deposit.chain_id());
		msg_channel.set_msg_type(protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_DEPOSIT);
		msg_channel.set_msg_data(deposit.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(msg_channel);

	}

	void MessageHandler::OnHandleDeposit(const protocol::MessageChannel &message_channel){
		AccountFrm::pointer account_ptr;
		if (!Environment::AccountFromDB(source_address_, account_ptr)) {
			LOG_ERROR("Address:%s not exsit", source_address_.c_str());
			return ;
		}
		cur_nonce_ = account_ptr->GetAccountNonce() + 1;

		protocol::MessageChannelDeposit deposit;
		protocol::ERRORCODE error_code = protocol::ERRCODE_SUCCESS;
		std::string error_desc = "";

		if (!deposit.ParseFromString(message_channel.msg_data())){
			error_desc = utils::String::Format("Parse MessageChannelQueryHead error!");
			error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}

		std::vector<std::string> send_para_list;
		Json::Value deposit_data = bumo::Proto2Json(deposit);
		Json::Value input_value;
		Json::Value params;

		params["chain_id"] = deposit.chain_id();
		params["deposit_data"] = deposit_data;
		params["hash"] = HashWrapper::Crypto(deposit.SerializeAsString());
		input_value["method"] = "deposit";
		input_value["params"] = params;
		send_para_list.push_back(input_value.toFastString());
		std::string hash;
		SendTransaction(send_para_list, hash);
		
	}

	void MessageHandler::SendTransaction(const std::vector<std::string> &paras, std::string& hash){
		int32_t err_code = 0;
		int32_t retry_time = 10;
		for (int i = 0; i <= retry_time; i++){
			std::string private_key = Configure::Instance().ledger_configure_.validation_privatekey_;
			TransactionFrm::pointer trans = CrossUtils::BuildTransaction(private_key, General::CONTRACT_CPC_ADDRESS, paras, cur_nonce_);
			if (nullptr == trans){
				LOG_ERROR("Trans pointer is null");
				continue;
			}
			hash = utils::String::BinToHexString(trans->GetContentHash().c_str());
			err_code = CrossUtils::SendTransaction(trans);
			switch (err_code)
			{
			case protocol::ERRCODE_SUCCESS:
			case  protocol::ERRCODE_ALREADY_EXIST:{
				cur_nonce_++;
				return;
			}
			case protocol::ERRCODE_BAD_SEQUENCE:{
				cur_nonce_++;
				break;
			}
			default:{
				LOG_ERROR("Send transaction erro code:%d", err_code);
				break;
			}
			}
		}
		return;
	}

	void MessageHandler::SendChildGenesesRequest(){
		protocol::MessageChannelChildGenesesRequest child_chain_request;
		child_chain_request.set_chain_id(General::GetSelfChainId());

		protocol::MessageChannel message_channel;
		message_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
		message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_CHILD_GENESES_REQUEST);
		message_channel.set_msg_data(child_chain_request.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(message_channel);
	}
}
