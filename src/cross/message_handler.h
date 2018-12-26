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
#define MSG_BUFFER_PERIOD 10
#define MSG_UPDATE_PERIOD 4
namespace bumo {
	
	typedef std::function<void(const protocol::MessageChannel &message_channel)> MessageChannelPoc;
	typedef std::map<int64_t, MessageChannelPoc> MessageChannelPocMap;

	class MessageHandlerBase :public IMessageChannelConsumer, public utils::Runnable{
	public:
		MessageHandlerBase();
		virtual ~MessageHandlerBase();
		bool Initialize();
		bool Exit();
		virtual void MessageChannelHandle(const protocol::MessageChannel &message_channel) = 0;
	protected:
		virtual void Run(utils::Thread *thread) override;
	private:
		virtual void HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel) override;
		virtual void CopyBufferMsgChannel() final;
		virtual void HandleMsgUpdate() ;
	private:
		bool enabled_;
		utils::Thread *thread_ptr_;
		utils::Mutex msg_channel_list_lock_;
		utils::Mutex msg_channel_buffer_list_lock_;
		std::list<protocol::MessageChannel> msg_channel_list_;
		std::list<protocol::MessageChannel> msg_channel_buffer_list_;
		int64_t last_update_time_;
		int64_t last_buffer_time_;
	};

	class MessageHandlerMainChain :public MessageHandlerBase{
	public:
		MessageHandlerMainChain();
		virtual ~MessageHandlerMainChain();
		bool HandlerInitialize();
		bool HandlerExit();
	private:
		virtual void MessageChannelHandle(const protocol::MessageChannel &message_channel) override;
		void OnHandleChildGenesesRequest(const protocol::MessageChannel &message_channel);
		void OnHandleWithdrawal(const protocol::MessageChannel &message_channel);
		void OnHandleQuerySubmitHead(const protocol::MessageChannel &message_channel);
		void OnHandleQueryWithdrawal(const protocol::MessageChannel &message_channel);
		void QuerySubmitHead(const int64_t &chain_id, const int64_t &seq, const std::string &hash, protocol::MessageChannelSubmitHead &submit_header);

	private:
		MessageChannelPocMap proc_methods_;
	};

	class MessageHandlerChildChain :public MessageHandlerBase{
	public:
		MessageHandlerChildChain();
		virtual ~MessageHandlerChildChain();
		bool HandlerInitialize();
		bool HandlerExit();
	private:
		bool CheckForChildBlock();
		virtual void MessageChannelHandle(const protocol::MessageChannel &message_channel) override;
		void OnHandleCreateChildChain(const protocol::MessageChannel &message_channel);
		void OnHandleChildGenesesResponse(const protocol::MessageChannel &message_channel);
		void SendChildGenesesRequest();
		void OnHandleQueryHead(const protocol::MessageChannel &message_channel);
		void CreateChildChain(const protocol::MessageChannelCreateChildChain &create_child_chain);
		void OnHandleChallenge(const protocol::MessageChannel &message_channel);

	private:
		int64_t last_deposit_time_;
		int64_t local_deposit_seq_;
		int64_t newest_deposit_seq_;

		bool init_;
		bool received_create_child_;
		MessageChannelPocMap proc_methods_;
	};

	class MessageHandler : public utils::Singleton<MessageHandler> {
		friend class utils::Singleton<bumo::MessageHandler>;
	public:
		MessageHandler();
		~MessageHandler();

		bool Initialize();
		bool Exit();

	private:
		std::shared_ptr<MessageHandlerMainChain> message_handler_main_chain_;
		std::shared_ptr<MessageHandlerChildChain> message_handler_child_chain_;

	};
}

#endif
