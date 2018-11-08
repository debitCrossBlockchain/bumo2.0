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
	class MessageChannelConsumer;
	class MessageChannelPeer :public Connection{

	public:
		MessageChannelPeer(server *server_h, client *client_h, tls_server *tls_server_h, tls_client *tls_client_h, connection_hdl con, const std::string &uri, int64_t id);
		~MessageChannelPeer();

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
		bool SendHello(const std::string &node_address, const int64_t &network_id, std::error_code &ec);

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
		bool OnMessageChannel(protocol::WsMessage &message, int64_t conn_id);


		void BroadcastMsg(int64_t type, const std::string &data);
		void BroadcastChainTxMsg(const protocol::MessageChannel& txMsg);

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
		bool ChainExist(int64_t peer_id, int64_t chain_id);
		bool CheckSameChain(int64_t local_chain_id, int64_t target_chain_id);
		int64_t GetChainIdFromConn(int64_t conn_id);

		//croos message hander
	public:
		virtual void AddConsumer(MessageChannelConsumer *msg_consumer, int64_t msg_type);
		virtual void RemoveConsumer(MessageChannelConsumer *msg_consumer, int64_t msg_type);
		virtual void Notify(const protocol::MessageChannel &message_channel);

	private:
		std::multimap<int64_t, MessageChannelConsumer*> message_channel_consumer_;
		utils::Mutex message_channel_consumer_lock_;
	};


	//croos message hander
	class MessageChannelConsumer{
	public:
		virtual void RegisterMessageChannelConsumer(MessageChannel *message_channel_event, int64_t msg_type);
		virtual void UnregisterMessageChannelConsumer(MessageChannel *message_channel_event, int64_t msg_type);
		virtual void HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel) = 0;
	};



}
#endif

