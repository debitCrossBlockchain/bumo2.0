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
	}

	MessageHandler::~MessageHandler(){
	}

	bool MessageHandler::Initialize(){
		proc_methods_[protocol::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN] = std::bind(&MessageHandler::OnHandleCreateChildChain, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_CHILD_GENESES_REQUEST] = std::bind(&MessageHandler::OnHandleChildGenesesRequest, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_CHILD_GENESES_RESPONSE] = std::bind(&MessageHandler::OnHandleChildGenesesResponse, this, std::placeholders::_1);

		MessageChannel &message_channel = MessageChannel::Instance();
		message_channel.RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN);
		message_channel.RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_CHILD_GENESES_REQUEST);
		message_channel.RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_CHILD_GENESES_RESPONSE);

		if (!CheckForChildBlock()){
			return false;
		}

		init_ = true;
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
		if (!child_chain_request.ParseFromString(message_channel.msg_data())){
			LOG_ERROR("Parse MessageChannelChildGenesesRequest error!");
			return;
		}

		if (child_chain_request.chain_id() <= 0){
			LOG_ERROR("Parse MessageChannelChildGenesesRequest error,invalid chain id(" FMT_I64 ")", child_chain_request.chain_id());
			return;
		}

#if 0
		std::string result;
		Json::Value input_value;
		Json::Value params;
		input_value["method"] = "queryChildChainInfo";
		input_value["params"]["chain_id"] = child_chain_request.chain_id();
		if (protocol::ERRCODE_SUCCESS != bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input_value.toFastString(), result)){
			LOG_ERROR("Query contract error!%s", result.c_str());
			return;
		}

		Json::Value json_result = Json::Value(Json::objectValue);
		json_result.fromString(result);
		if (!json_result.isArray()){
			LOG_ERROR("Query contract error! Json result is not array.%s", json_result.toFastString().c_str());
			return;
		}
		Json::Value custom_result;
		custom_result.fromString(json_result[Json::UInt(0)]["result"]["value"].asString());
#endif

		protocol::MessageChannelCreateChildChain create_child_chain;
		std::string error_msg;
#if 0
		if (!Json2Proto(custom_result, create_child_chain, error_msg)){
				LOG_ERROR("Invalid contract result:%s", custom_result.toFastString());
				return;
		}
#endif

		protocol::MessageChannelChildGenesesResponse response;
		response.set_error_code(protocol::ERRCODE_SUCCESS);
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

		if (1){
			//Debug
			GenesisConfigure &genesis_config = Configure::Instance().genesis_configure_;
			genesis_config.account_ = "buQYgZmzaXAjFovGWnbqZUsA6vpSnySs8Z2b";
			genesis_config.fees_.base_reserve_ = 1000;
			genesis_config.fees_.gas_price_ = 1000;
			genesis_config.slogan_ = "Hello, child chain!";
			genesis_config.validators_.push_back("buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY");
		}
		else{
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
		}

		LOG_INFO("Received create child chain message, validators:%s, account:%s, slogan:%s, chain id:(" FMT_I64 ")",
			validators.c_str(), create_child_chain.genesis_account().c_str(),
			create_child_chain.slogan().c_str(), create_child_chain.chain_id());

		received_create_child_ = true;
	}

	void MessageHandler::SendChildGenesesRequest(){
#if 1
		if (General::GetSelfChainId() <= General::MAIN_CHAIN_ID){
			LOG_ERROR("The main chain program cannot send this message.");
			return;
		}
		protocol::MessageChannelCreateChildChain create_child_chain;
		create_child_chain.set_genesis_account("abc");

		protocol::MessageChannelChildGenesesRequest request;
		request.set_chain_id(1);

		//Push message to child chain.
		protocol::MessageChannel message;
		message.set_target_chain_id(General::MAIN_CHAIN_ID);
		message.set_msg_type(protocol::MESSAGE_CHANNEL_CHILD_GENESES_REQUEST);
		message.set_msg_data(request.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(message);
#endif

#if 0
		protocol::MessageChannelChildGenesesRequest child_chain_request;
		child_chain_request.set_chain_id(General::GetSelfChainId());

		protocol::MessageChannel message_channel;
		message_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
		message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_CHILD_GENESES_REQUEST);
		message_channel.set_msg_data(child_chain_request.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(message_channel);
#endif
	}
}
