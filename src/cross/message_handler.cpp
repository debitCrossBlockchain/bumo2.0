#include <common/storage.h>
#include "message_handler.h"

namespace bumo {
	extern bool g_enable_;

	MessageHandler::MessageHandler(){
		init_ = false;
		received_create_child__ = false;
	}

	MessageHandler::~MessageHandler(){
	}

	bool MessageHandler::Initialize(){
		if (!CheckForChildBlock()){
			return false;
		}
		init_ = true;
		return true;
	}

	bool MessageHandler::Exit(){

		return true;
	}

	bool MessageHandler::CommitMessage(){

		if (1){
			//Recv create child message
			GenesisConfigure &genesis_config = Configure::Instance().genesis_configure_;
			genesis_config.account_ = "buQYgZmzaXAjFovGWnbqZUsA6vpSnySs8Z2b";
			genesis_config.fees_.base_reserve_ = 1000;
			genesis_config.fees_.gas_price_ = 1000;
			genesis_config.slogan_ = "Hello, child chain!";
			genesis_config.validators_.push_back("buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY");
			received_create_child__ = true;
		}

		return true;
	}

	bool MessageHandler::CheckForChildBlock(){
		if (General::GetSelfChainId() <= 0) {
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
			LOG_STD_ERR("Other parameters of the child chain must be configured to be empty");
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
				CommitMessage();
				last_request_time = cur_time;
			}

			if (received_create_child__){
				LOG_INFO("Received create child (" FMT_I64 ") chain message..", General::GetSelfChainId());
				break;
			}

			utils::Sleep(1);
		}

		return true;
	}
}
