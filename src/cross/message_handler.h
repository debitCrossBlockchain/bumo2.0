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

#ifndef MESSAGE_HANDLER_H_
#define MESSAGE_HANDLER_H_

#include <utils/singleton.h>
#include <utils/thread.h>
#include "message_channel_manager.h"
#include <cross/cross_utils.h>


#define DEPOSIT_QUERY_PERIOD 10

namespace bumo {

	typedef std::function<void(const protocol::MessageChannel &message_channel)> MessageChannelPoc;
	typedef std::map<int64_t, MessageChannelPoc> MessageChannelPocMap;

	class MessageHandler : public utils::Singleton<MessageHandler>, public IMessageChannelConsumer,
		public ITransactionSenderNotify,public utils::Runnable{
		friend class utils::Singleton<bumo::MessageHandler>;
	public:
		MessageHandler();
		~MessageHandler();

		bool Initialize();
		bool Exit();

	private:
		virtual void Run(utils::Thread *thread) override;

		bool CheckForChildBlock();
		virtual void HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel) override;
		virtual void HandleTransactionSenderResult(const TransTask &task_task, const TransTaskResult &task_result) override;

		void OnHandleCreateChildChain(const protocol::MessageChannel &message_channel);
		void OnHandleChildGenesesRequest(const protocol::MessageChannel &message_channel);
		void OnHandleChildGenesesResponse(const protocol::MessageChannel &message_channel);
		void OnHandleQueryHead(const protocol::MessageChannel &message_channel);
		void OnHandleQueryDeposit(const protocol::MessageChannel &message_channel);
		void OnHandleDeposit(const protocol::MessageChannel &message_channel);

		void CreateChildChain(const protocol::MessageChannelCreateChildChain &create_child_chain);
		void SendChildGenesesRequest();
		//void SendTransaction(const std::vector<std::string> &paras, std::string& hash);

		void PullLostDeposit();

	private:
		bool enabled_;
		utils::Thread *thread_ptr_;
		int64_t last_deposit_time_;
		int64_t deposit_seq_;

		bool init_;
		bool received_create_child_;
		MessageChannelPocMap proc_methods_;

	};
}

#endif
