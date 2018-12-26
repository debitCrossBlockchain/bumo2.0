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

#ifndef BASE_TRADER_H_
#define BASE_TRADER_H_

#include <utils/singleton.h>
#include <utils/thread.h>
#include <cross/cross_utils.h>
#include<cross/message_channel_manager.h>
#define PROPOSER_PERIOD 15

namespace bumo {

	class BaseProposer :
		public bumo::IMessageChannelConsumer, public ITransactionSenderNotify,
		public utils::Runnable{

	public:
		BaseProposer();
		virtual ~BaseProposer();

		bool Initialize(bool user_proposer);
		bool Exit();

	protected:
		virtual void Run(utils::Thread *thread) override;
		virtual void HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel) override;
		virtual void HandleTransactionSenderResult(const TransTask &task_task, const TransTaskResult &task_result) override;

		virtual void DoTimerUpdate() = 0;
		virtual void DoHandleMessageChannel(const protocol::MessageChannel &message_channel) = 0;
		virtual void DoHandleSenderResult(const TransTask &task_task, const TransTaskResult &task_result) = 0;

		void BreakProposer(const std::string &error_des);
	private:
		virtual void CopyBufferMsgChannel() final;
		virtual void HandleMsgUpdate();
	protected:
		bool enabled_;
		utils::Thread *thread_ptr_;

		int64_t last_propose_time_;
		bool use_proposer_;
		std::string source_address_;
	
	private:
		utils::Mutex msg_channel_list_lock_;
		utils::Mutex msg_channel_buffer_list_lock_;
		std::list<protocol::MessageChannel> msg_channel_list_;
		std::list<protocol::MessageChannel> msg_channel_buffer_list_;
		int64_t last_update_time_;
		int64_t last_buffer_time_;
	};
}

#endif
