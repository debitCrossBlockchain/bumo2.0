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
	class MessageChannelPeer :public Connection{

	public:
		MessageChannelPeer(server *server_h, client *client_h, tls_server *tls_server_h, tls_client *tls_client_h, connection_hdl con, const std::string &uri, int64_t id);
		~MessageChannelPeer();
		bool Set(const protocol::ChainSubscribeTx &sub);
		bool Filter(const protocol::TransactionEnvStore &tx_msg);

	private:

		//Peer infomation
		std::set<std::string> tx_filter_address_;
		int64_t active_time_;
		int64_t delay_;

		//Peer infomation
		std::string peer_node_address_;
		int64_t chain_id_;
	public:
		bool IsActive() const;
		std::string GetPeerNodeAddress() const;
		int64_t GetActiveTime() const;
		int64_t GetDelay() const;
		int64_t GetChainId() const;

		bool SendPeers(std::error_code &ec);
		void SetPeerInfo(const protocol::MessageChannelHello &hello);
		void SetActiveTime(int64_t current_time);
		bool SendHello(const std::string &node_address, const int64_t &network_id,std::error_code &ec);

		virtual void ToJson(Json::Value &status) const;
		virtual bool OnNetworkTimer(int64_t current_time);
	};

	class MessageChannel :public utils::Singleton<MessageChannel>,
		public TimerNotify,
		public StatusModule,
		public Network,
		public utils::Runnable {
		friend class utils::Singleton<bumo::MessageChannel>;
	public:
		MessageChannel();
		~MessageChannel();


		//virtual bool Send(const ZMQTaskType type, const std::string& buf);

		bool Initialize(MessageChannelConfigure & message_channel_configure);
		bool Exit();

		// Handlers
		bool OnHello(protocol::WsMessage &message, int64_t conn_id);
		bool OnCrateChildChain(protocol::WsMessage &message, int64_t conn_id);
		bool OnMainChainMix(protocol::WsMessage &message, int64_t conn_id);
		bool OnChildChainMix(protocol::WsMessage &message, int64_t conn_id);
		bool OnDeposit(protocol::WsMessage &message, int64_t conn_id);
		bool OnWithdrawal(protocol::WsMessage &message, int64_t conn_id);
		bool OnFastWithdrawal(protocol::WsMessage &message, int64_t conn_id);
		bool OnSubmitChildHead(protocol::WsMessage &message, int64_t conn_id);
		bool OnChallengeWithdrawal(protocol::WsMessage &message, int64_t conn_id);
		bool OnChallengeChildHead(protocol::WsMessage &message, int64_t conn_id);
		bool OnChildChainGeneses(protocol::WsMessage &message, int64_t conn_id);
		bool OnSubmitTransaction(protocol::WsMessage &message, int64_t conn_id);
		bool OnSubscribeTx(protocol::WsMessage &message, int64_t conn_id);

		void BroadcastMsg(int64_t type, const std::string &data);
		void BroadcastChainTxMsg(const protocol::TransactionEnvStore& txMsg);

		virtual void OnDisconnect(Connection *conn);
		virtual bool OnConnectOpen(Connection *conn);
		virtual Connection *CreateConnectObject(server *server_h, client *client_,
			tls_server *tls_server_h, tls_client *tls_client_h,
			connection_hdl con, const std::string &uri, int64_t id);
		virtual bool OnVerifyCallback(bool preverified, asio::ssl::verify_context& ctx);
		virtual void GetModuleStatus(Json::Value &data);
	protected:
		virtual void Run(utils::Thread *thread) override;


	private:
		utils::Thread *thread_ptr_;

		uint64_t last_connect_time_;
		uint64_t connect_interval_;
		int32_t  total_peers_count_;
		int64_t  network_id_;
		std::error_code last_ec_;
		int64_t last_uptate_time_;


		//client
	public:
		bool ConnectToMessageChannel();
		bool SendRequest(int64_t id, int64_t type, const std::string &data);
		bool ReceiveMsg(int64_t type, const std::string &data, int64_t id);
		virtual void OnTimer(int64_t current_time) override;
		virtual void OnSlowTimer(int64_t current_time) override {};



	};
}
#endif

