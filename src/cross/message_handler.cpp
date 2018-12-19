#include <common/storage.h>
#include <common/private_key.h>
#include <common/pb2json.h>
#include<api/websocket_server.h>
#include<cross/challenge_manager.h>
#include "message_handler.h"


namespace bumo {
	extern bool g_enable_;

	MessageHandlerBase::MessageHandlerBase() :
		enabled_(false),
		thread_ptr_(NULL){
		last_update_time_ = utils::Timestamp::HighResolution();
		last_buffer_time_ = utils::Timestamp::HighResolution();
	}
	MessageHandlerBase::~MessageHandlerBase(){
		if (thread_ptr_){
			delete thread_ptr_;
			thread_ptr_ = NULL;
		}
	}

	bool MessageHandlerBase::Initialize() {
		enabled_ = true;
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("MessageHandlerManager")) {
			return false;
		}
		MessageChannel &message_channel = MessageChannel::Instance();
		for (int64_t i = 1; i <= 19; i++)
		{
			message_channel.RegisterMessageChannelConsumer(this, i);
		}
		return true;
	}

	bool MessageHandlerBase::Exit(){
		enabled_ = false;
		if (thread_ptr_) {
			thread_ptr_->JoinWithStop();
		}
		MessageChannel &message_channel = MessageChannel::Instance();
		for (int64_t i = 1; i <= 19; i++)
		{
			message_channel.UnregisterMessageChannelConsumer(this, i);
		}
		return true;
	}

	void MessageHandlerBase::Run(utils::Thread *thread) {
		while (enabled_){
			utils::Sleep(10);
			int64_t current_time = utils::Timestamp::HighResolution();
			if ((current_time - last_buffer_time_) > MSG_BUFFER_PERIOD * utils::MICRO_UNITS_PER_SEC){
				CopyBufferMsgChannel();
				last_buffer_time_ = current_time;
			}

			if ((current_time - last_update_time_) > MSG_UPDATE_PERIOD * utils::MICRO_UNITS_PER_SEC){
				HandleMsgUpdate();
				last_update_time_ = current_time;
			}
		}
	}

	void MessageHandlerBase::HandleMsgUpdate(){
		utils::MutexGuard guard(msg_channel_list_lock_);
		std::list<protocol::MessageChannel>::iterator iter = msg_channel_list_.begin();
		while (iter != msg_channel_list_.end()) {
			const protocol::MessageChannel &msg_channel = *iter;
			MessageChannelHandle(msg_channel);
			msg_channel_list_.erase(iter++);
			utils::Sleep(10);
		}
	}

	void MessageHandlerBase::HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel){
		if (!enabled_){
			return;
		}
		utils::MutexGuard guard(msg_channel_buffer_list_lock_);
		msg_channel_buffer_list_.push_back(message_channel);
	}

	void MessageHandlerBase::CopyBufferMsgChannel(){
		std::list<protocol::MessageChannel> msg_channel_list;
		{
			utils::MutexGuard guard(msg_channel_buffer_list_lock_);
			msg_channel_list.insert(msg_channel_list.end(), msg_channel_buffer_list_.begin(), msg_channel_buffer_list_.end());
			msg_channel_buffer_list_.clear();
		}

		utils::MutexGuard guard(msg_channel_list_lock_);
		std::list<protocol::MessageChannel>::const_iterator iter = msg_channel_list.begin();
		while (iter != msg_channel_list.end()){
			const protocol::MessageChannel &msg_channel = *iter;
			msg_channel_list_.push_back(msg_channel);
			iter++;
		}
	}

	MessageHandlerMainChain::MessageHandlerMainChain(){
	}

	MessageHandlerMainChain::~MessageHandlerMainChain(){

	}

	bool MessageHandlerMainChain::HandlerInitialize(){
		proc_methods_[protocol::MESSAGE_CHANNEL_CHILD_GENESES_REQUEST] = std::bind(&MessageHandlerMainChain::OnHandleChildGenesesRequest, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_WITHDRAWAL] = std::bind(&MessageHandlerMainChain::OnHandleWithdrawal, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_QUERY_SUBMIT_HEAD] = std::bind(&MessageHandlerMainChain::OnHandleQuerySubmitHead, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_CHILD_CHALLENGE_HEAD] = std::bind(&MessageHandlerMainChain::OnHandleQuerySubmitHead, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_CHILD_CHALLENGE_WITHDRAWAL] = std::bind(&MessageHandlerMainChain::OnHandleQuerySubmitHead, this, std::placeholders::_1);
		if (!Initialize()){
			return false;
		}
		return true;
	}

	bool MessageHandlerMainChain::HandlerExit(){
		if (!Exit()){
			return false;
		}
		return true;
	}

	void MessageHandlerMainChain::MessageChannelHandle(const protocol::MessageChannel &message_channel){
		MessageChannelPoc proc;
		MessageChannelPocMap::iterator iter = proc_methods_.find(message_channel.msg_type());
		if (iter == proc_methods_.end()) {
			LOG_TRACE("Type(" FMT_I64 ") not found", message_channel.msg_type());
			return;
		}

		iter->second(message_channel);
	}

	void MessageHandlerMainChain::OnHandleChildGenesesRequest(const protocol::MessageChannel &message_channel){
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

	void MessageHandlerMainChain::OnHandleWithdrawal(const protocol::MessageChannel &message_channel){
		protocol::MessageChannelWithdrawal withdrawal;
		if (General::GetSelfChainId() != General::MAIN_CHAIN_ID){
			return;
		}

		if (!withdrawal.ParseFromString(message_channel.msg_data())){
			int64_t error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("Parse MessageChannelWithdrawal error, err_code is (" FMT_I64 ")", error_code);
			return;
		}

		bumo::WebSocketServer::GetInstance()->BroadcastMsg(protocol::EVENT_WITHDRAWAL, withdrawal.SerializeAsString());
	}

	void MessageHandlerMainChain::QuerySubmitHead(const int64_t &chain_id, const int64_t &seq, const std::string &hash, protocol::MessageChannelSubmitHead &submit_header){
		protocol::LedgerHeader ledger_header;
		submit_header.set_state(seq);
		auto header = submit_header.mutable_header();
		Json::Value params;
		params["chain_id"] = chain_id;
		if (seq == -1){
			params["header_hash"] = "";
		}
		else{
			params["header_hash"] = hash;
		}

		Json::Value input_value;
		input_value["method"] = "queryChildBlockHeader";
		input_value["params"] = params;

		Json::Value result_list;
		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input_value.toFastString(), result_list);
		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		Json::Value object;
		object.fromString(result.c_str());
		if (error_code != protocol::ERRCODE_SUCCESS){
			LOG_ERROR("Failed to query child block .%d", error_code);
			return;
		}

		std::string error_msg;
		if (!bumo::Json2Proto(object, *header, error_msg)) {
			LOG_ERROR("block_header Failed to Json2Proto error_msg=%s", error_msg.c_str());
			return;
		}
	}


	void MessageHandlerMainChain::OnHandleQuerySubmitHead(const protocol::MessageChannel &message_channel){
		if (General::GetSelfChainId() != General::MAIN_CHAIN_ID){
			return;
		}
		protocol::MessageChannelQuerySubmitHead query_submit_head;
		if (!query_submit_head.ParseFromString(message_channel.msg_data())){
			int64_t error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("Parse MessageChannelQuerySubmitHead error, err_code is (" FMT_I64 ")", error_code);
			return;
		}
		protocol::MessageChannel msg_channel;
		protocol::MessageChannelSubmitHead msg_submit_head;
	
		Json::Value query_head = bumo::Proto2Json(query_submit_head);
		QuerySubmitHead(query_submit_head.chain_id(), query_submit_head.seq(), query_head["hash"].asString(), msg_submit_head);
		
		
		msg_channel.set_msg_type(protocol::MESSAGE_CHANNEL_CHALLENGE_HEAD);
		msg_channel.set_target_chain_id(query_submit_head.chain_id());
		msg_channel.set_msg_data(msg_submit_head.SerializeAsString());
		bumo::MessageChannel::GetInstance()->MessageChannelProducer(msg_channel);
	}

	void MessageHandlerMainChain::OnHandleQueryWithdrawal(const protocol::MessageChannel &message_channel){
		if (General::GetSelfChainId() != General::MAIN_CHAIN_ID){
			return;
		}

		protocol::MessageChannelQueryWithdrawal query_withdrawal;
		if (!query_withdrawal.ParseFromString(message_channel.msg_data())){
			int64_t error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("Parse MessageChannelQueryWithdrawal error, err_code is (" FMT_I64 ")", error_code);
			return;
		}

		Json::Value params;
		params["chain_id"] = query_withdrawal.chain_id();
		if (query_withdrawal.seq() == -1){
			params["seq"] = "";
		}
		else{
			params["seq"] = query_withdrawal.seq();
		}

		Json::Value input_value;
		input_value["method"] = "queryChildWithdrawal";
		input_value["params"] = params;

		Json::Value result_list;
		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input_value.toFastString(), result_list);
		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		Json::Value object;
		object.fromString(result.c_str());
		if (error_code != protocol::ERRCODE_SUCCESS){
			LOG_ERROR("Failed to query child block .%d", error_code);
			return;
		}

		std::string error_msg;
		protocol::MessageChannelWithdrawalChallenge  withdrawal_challenge;
		if (!bumo::Json2Proto(object["block_header"], withdrawal_challenge, error_msg)) {
			LOG_ERROR("block_header Failed to Json2Proto error_msg=%s", error_msg.c_str());
			return;
		}

		protocol::MessageChannel msg_channel;

		msg_channel.set_msg_type(protocol::MESSAGE_CHANNEL_CHALLENGE_WITHDRAWAL);
		msg_channel.set_target_chain_id(query_withdrawal.chain_id());
		msg_channel.set_msg_data(withdrawal_challenge.SerializeAsString());
		bumo::MessageChannel::GetInstance()->MessageChannelProducer(msg_channel);
	}

	void MessageHandlerMainChain::OnHandleChildChallengeSubmitHead(const protocol::MessageChannel &message_channel){

	}

	void MessageHandlerMainChain::OnHandleChildChallengeWithdrawal(const protocol::MessageChannel &message_channel){

	}


	MessageHandlerChildChain::MessageHandlerChildChain(){
		init_ = false;
		received_create_child_ = false;
		last_deposit_time_ = utils::Timestamp::HighResolution();
		local_deposit_seq_ = 0;
		newest_deposit_seq_ = 0;
	}

	MessageHandlerChildChain::~MessageHandlerChildChain(){

	}

	bool MessageHandlerChildChain::HandlerInitialize(){
		proc_methods_[protocol::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN] = std::bind(&MessageHandlerChildChain::OnHandleCreateChildChain, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_CHILD_GENESES_RESPONSE] = std::bind(&MessageHandlerChildChain::OnHandleChildGenesesResponse, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_QUERY_HEAD] = std::bind(&MessageHandlerChildChain::OnHandleQueryHead, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_CHALLENGE_HEAD] = std::bind(&MessageHandlerChildChain::OnHandleChallenge, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_CHALLENGE_WITHDRAWAL] = std::bind(&MessageHandlerChildChain::OnHandleChallenge, this, std::placeholders::_1);
		if (!Initialize()){
			return false;
		}

		if (!CheckForChildBlock()){
			return false;
		}

		init_ = true;

		return true;
	}

	bool MessageHandlerChildChain::HandlerExit(){
		if (!Exit()){
			return false;
		}
		init_ = false;
		return true;
	}

	void MessageHandlerChildChain::MessageChannelHandle(const protocol::MessageChannel &message_channel){
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

	bool MessageHandlerChildChain::CheckForChildBlock(){
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

	void MessageHandlerChildChain::OnHandleCreateChildChain(const protocol::MessageChannel &message_channel){
		protocol::MessageChannelCreateChildChain create_child_chain;
		if (!create_child_chain.ParseFromString(message_channel.msg_data())){
			LOG_ERROR("Parse MessageChannelCreateChildChain error!");
			return;
		}

		CreateChildChain(create_child_chain);
		return;
	}

	

	void MessageHandlerChildChain::OnHandleChildGenesesResponse(const protocol::MessageChannel &message_channel){
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

	void MessageHandlerChildChain::CreateChildChain(const protocol::MessageChannelCreateChildChain &create_child_chain){
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

	void MessageHandlerChildChain::OnHandleQueryHead(const protocol::MessageChannel &message_channel){
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
		const protocol::LedgerHeader& ledger_header = frm.GetProtoHeader();
		//Push message to child chain.
		protocol::MessageChannel msg_channel;
		msg_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
		msg_channel.set_msg_type(protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_SUBMIT_HEAD);
		msg_channel.set_msg_data(ledger_header.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(msg_channel);
	}

	void MessageHandlerChildChain::OnHandleChallenge(const protocol::MessageChannel &message_channel){
		bumo::ChallengeManager::Instance().ChallengeNotify(message_channel);
	}

	void MessageHandlerChildChain::SendChildGenesesRequest(){
		protocol::MessageChannelChildGenesesRequest child_chain_request;
		child_chain_request.set_chain_id(General::GetSelfChainId());

		protocol::MessageChannel message_channel;
		message_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
		message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_CHILD_GENESES_REQUEST);
		message_channel.set_msg_data(child_chain_request.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(message_channel);
	}

	MessageHandler::MessageHandler(){
	}

	MessageHandler::~MessageHandler(){
	}

	bool MessageHandler::Initialize(){
		if (General::GetSelfChainId() == General::MAIN_CHAIN_ID){
			message_handler_main_chain_ = std::make_shared<MessageHandlerMainChain>();
			return message_handler_main_chain_->HandlerInitialize();
		}
		else{
			message_handler_child_chain_ = std::make_shared<MessageHandlerChildChain>();
			return message_handler_child_chain_->HandlerInitialize();
		}

		return true;
	}

	bool MessageHandler::Exit(){
		if (General::GetSelfChainId() == General::MAIN_CHAIN_ID){
			return message_handler_main_chain_->HandlerExit();
		}
		else{
			return message_handler_child_chain_->HandlerExit();
		}
	}


}
