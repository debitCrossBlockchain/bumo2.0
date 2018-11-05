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

#ifndef MESSAGE_CHANNEL_MANAGER_H_
#define MESSAGE_CHANNEL_MANAGER_H_

#include <proto/cpp/chain.pb.h>
#include <proto/cpp/overlay.pb.h>
#include<main/configure.h>
#include <common/network.h>
#include <monitor/system_manager.h>

namespace bumo {
	class MessageChannelManager :public Connection{
	public:
		MessageChannelManager(server *server_h, client *client_h, tls_server *tls_server_h, tls_client *tls_client_h, connection_hdl con, const std::string &uri, int64_t id);
		~MessageChannelManager();
	};

	class MessageChannelServer :public utils::Singleton<MessageChannelServer>,
		public StatusModule,
		public Network,
		public utils::Runnable {
		friend class utils::Singleton<bumo::MessageChannelServer>;
	public:
		MessageChannelServer();
		~MessageChannelServer();


		//virtual bool Send(const ZMQTaskType type, const std::string& buf);

		bool Initialize(MessageChannelConfigure & message_channel_configure);
		bool Exit();

		// Handlers
		bool OnCrateChildChain(protocol::MessageChannel &message, int64_t conn_id);
		bool OnMainChainMix(protocol::MessageChannel &message, int64_t conn_id);
		bool OnChildChainMix(protocol::MessageChannel &message, int64_t conn_id);
		bool OnDeposit(protocol::MessageChannel &message, int64_t conn_id);
		bool OnWithdrawal(protocol::MessageChannel &message, int64_t conn_id);
		bool OnFastWithdrawal(protocol::MessageChannel &message, int64_t conn_id);
		bool OnSubmitChildHead(protocol::MessageChannel &message, int64_t conn_id);
		bool OnChallengeWithdrawal(protocol::MessageChannel &message, int64_t conn_id);
		bool OnChallengeChildHead(protocol::MessageChannel &message, int64_t conn_id);
		bool OnChildChainGeneses(protocol::MessageChannel &message, int64_t conn_id);
		bool OnSubmitTransaction(protocol::MessageChannel &message, int64_t conn_id);
		bool OnSubscribeTx(protocol::MessageChannel &message, int64_t conn_id);

		void BroadcastMsg(int64_t type, const std::string &data);
		void BroadcastChainTxMsg(const protocol::TransactionEnvStore& txMsg);

		virtual Connection *CreateConnectObject(server *server_h, client *client_,
			tls_server *tls_server_h, tls_client *tls_client_h,
			connection_hdl con, const std::string &uri, int64_t id);

		virtual void GetModuleStatus(Json::Value &data);
	protected:
		virtual void Run(utils::Thread *thread) override;

	private:
		utils::Thread *thread_ptr_;

		uint64_t last_connect_time_;
		uint64_t connect_interval_;
	};
}
#endif

