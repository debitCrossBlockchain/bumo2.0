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

namespace bumo {

	typedef std::function<void(const protocol::MessageChannel &message_channel)> MessageChannelPoc;
	typedef std::map<int64_t, MessageChannelPoc> MessageChannelPocMap;

	class MessageHandler : public utils::Singleton<MessageHandler>, public IMessageChannelConsumer {
		friend class utils::Singleton<bumo::MessageHandler>;
	public:
		MessageHandler();
		~MessageHandler();

		bool Initialize();
		bool Exit();

	private:
		bool InitDepositSeq();
		bool CheckForChildBlock();
		virtual void HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel) override;

		void OnHandleCreateChildChain(const protocol::MessageChannel &message_channel);
		void OnHandleChildGenesesRequest(const protocol::MessageChannel &message_channel);
		void OnHandleChildGenesesResponse(const protocol::MessageChannel &message_channel);
		void OnHandleQueryHead(const protocol::MessageChannel &message_channel); 

		void OnHandleWithdrawal(const protocol::MessageChannel &message_channel);
		void CreateChildChain(const protocol::MessageChannelCreateChildChain &create_child_chain);
		void SendChildGenesesRequest();

	private:
		int64_t last_deposit_time_;
		int64_t local_deposit_seq_;
		int64_t newest_deposit_seq_;

		bool init_;
		bool received_create_child_;
		MessageChannelPocMap proc_methods_;

	};
}

#endif
