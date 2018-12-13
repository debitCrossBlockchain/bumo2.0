/*
bumo is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

bumo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with bumo.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cross/base_proposer.h>
#include <cross/cross_utils.h>
#include <ledger/ledger_manager.h>
#include <glue/glue_manager.h>
#include <overlay/peer_manager.h>
#include <algorithm>
namespace bumo {
	BaseProposer::BaseProposer() :
		enabled_(false),
		thread_ptr_(NULL){
		last_propose_time_ = utils::Timestamp::HighResolution();
		use_proposer_ = false;
	}
	BaseProposer::~BaseProposer(){
		if (thread_ptr_){
			delete thread_ptr_;
			thread_ptr_ = NULL;
		}
	}

	bool BaseProposer::Initialize(bool user_proposer){
		use_proposer_ = user_proposer;

		if (!use_proposer_){
			return true;
		}

		PrivateKey private_key(Configure::Instance().ledger_configure_.validation_privatekey_);
		if (!private_key.IsValid()){
			LOG_ERROR("Private key is not valid");
			return false;
		}

		source_address_ = private_key.GetEncAddress();

		enabled_ = true;
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("ProposerManager")) {
			return false;
		}
		return true;
	}

	bool BaseProposer::Exit(){
		if (!use_proposer_){
			return true;
		}

		enabled_ = false;
		if (thread_ptr_) {
			thread_ptr_->JoinWithStop();
		}

		return true;
	}

	void BaseProposer::Run(utils::Thread *thread) {
		while (enabled_){
			utils::Sleep(10);
			int64_t current_time = utils::Timestamp::HighResolution();
			if ((current_time - last_propose_time_) < PROPOSER_PERIOD * utils::MICRO_UNITS_PER_SEC){
				continue;
			}

			DoTimerUpdate();
			last_propose_time_ = current_time;
		}
	}

	void BaseProposer::HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel){
		if (!enabled_){
			return;
		}

		DoHandleMessageChannel(message_channel);
	}

	void BaseProposer::HandleTransactionSenderResult(const TransTask &task_task, const TransTaskResult &task_result){
		if (!enabled_){
			return;
		}

		if (!task_result.result_){
			BreakProposer(task_result.desc_);
			return;
		}

		DoHandleSenderResult(task_task, task_result);
	}

	void BaseProposer::BreakProposer(const std::string &error_des){
		enabled_ = false;
		assert(false);
		LOG_ERROR("%s", error_des.c_str());
	}
}