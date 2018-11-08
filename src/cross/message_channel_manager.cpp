#include <utils/headers.h>
#include <common/general.h>
#include <main/configure.h>
#include <proto/cpp/monitor.pb.h>
#include <overlay/peer_manager.h>
#include <glue/glue_manager.h>
#include <ledger/ledger_manager.h>
#include <monitor/monitor.h>

#include "message_channel_manager.h"
using namespace std;

namespace bumo {

	MessageChannelPeer::MessageChannelPeer(server *server_h, client *client_h, tls_server *tls_server_h, tls_client *tls_client_h, connection_hdl con, const std::string &uri, int64_t id) :
		Connection(server_h, client_h, tls_server_h, tls_client_h, con, uri, id) {
		active_time_ = 0;
		delay_ = 0;
		chain_id_ = 0;

	}


	MessageChannelPeer::~MessageChannelPeer(){
	}


	std::string MessageChannelPeer::GetPeerNodeAddress() const {
		return peer_node_address_;
	}

	int64_t MessageChannelPeer::GetActiveTime() const {
		return active_time_;
	}

	bool MessageChannelPeer::IsActive() const {
		return active_time_ > 0;
	}


	void MessageChannelPeer::SetPeerInfo(const protocol::MessageChannelHello &hello) {

		peer_node_address_ = hello.node_address();
		chain_id_ = hello.chain_id();
	}

	void MessageChannelPeer::SetActiveTime(int64_t current_time) {
		active_time_ = current_time;
	}

	bool MessageChannelPeer::SendHello(const std::string &node_address, const int64_t &network_id, std::error_code &ec) {
		protocol::MessageChannelHello hello;


		hello.set_node_address(node_address);
		hello.set_network_id(network_id);
		hello.set_chain_id(General::GetSelfChainId());
		return SendRequest(protocol::MESSAGE_CHANNEL_NODE_HELLO, hello.SerializeAsString(), ec);
	}

	void MessageChannelPeer::ToJson(Json::Value &status) const {
		Connection::ToJson(status);

		status["node_address"] = peer_node_address_;
		status["delay"] = delay_;
		status["active"] = IsActive();
		status["active_time"] = active_time_;
	}

	int64_t MessageChannelPeer::GetDelay() const {
		return delay_;
	}

	int64_t MessageChannelPeer::GetChainId() const{
		return chain_id_;
	}

	bool MessageChannelPeer::OnNetworkTimer(int64_t current_time) {
		if (!IsActive() && current_time - connect_start_time_ > 10 * utils::MICRO_UNITS_PER_SEC) {
			LOG_ERROR("Failed to check peer active, (%s) timeout", GetPeerAddress().ToIpPort().c_str());
			return false;
		}

		return true;
	}


	MessageChannel::MessageChannel() : Network(SslParameter()) {
		connect_interval_ = 120 * utils::MICRO_UNITS_PER_SEC;
		last_connect_time_ = 0;
		last_uptate_time_ = utils::Timestamp::HighResolution();

		request_methods_[protocol::MESSAGE_CHANNEL_NODE_HELLO] = std::bind(&MessageChannel::OnHello, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_NODE_PACKAGE] = std::bind(&MessageChannel::OnMessageChannel, this, std::placeholders::_1, std::placeholders::_2);
		thread_ptr_ = NULL;
	}

	MessageChannel::~MessageChannel() {
		if (thread_ptr_){
			delete thread_ptr_;
		}
	}

	bool MessageChannel::Initialize(MessageChannelConfigure & message_channel_configure) {
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("messageChannel")) {
			return false;
		}
		StatusModule::RegisterModule(this);
		TimerNotify::RegisterModule(this);
		LOG_INFO("Initialized message channel server successfully");
		return true;
	}

	bool MessageChannel::Exit() {
		Stop();
		thread_ptr_->JoinWithStop();
		return true;
	}

	void MessageChannel::Run(utils::Thread *thread) {
		if (General::GetSelfChainId() == 0){
			Start(bumo::Configure::Instance().message_channel_configure_.listen_address_);
		}
		else{
			utils::InetAddress ip;
			Start(ip);
		}
	}

	bool MessageChannel::OnHello(protocol::WsMessage &message, int64_t conn_id){

		protocol::MessageChannelHello hello;
		hello.ParseFromString(message.data());

		protocol::MessageChannelHelloResponse cmsg;
		std::error_code ignore_ec;

		utils::MutexGuard guard_(conns_list_lock_);
		Connection *conn = GetConnection(conn_id);
		do{
			if (!conn) {
				LOG_ERROR("MessageChannelPeer conn pointer is empty");
				return false;
			}

			MessageChannelPeer *peer = (MessageChannelPeer *)conn;
			peer->SetPeerInfo(hello);

			if (ChainExist(peer->GetId(), hello.chain_id())) {
				cmsg.set_error_code(protocol::ERRCODE_INVALID_PARAMETER);
				cmsg.set_error_desc(utils::String::Format("Duplicated connection with ip(%s), id(" FMT_I64 ")", peer->GetPeerAddress().ToIp().c_str(), peer->GetId()));
				LOG_ERROR("Failed to process the peer hello message.%s", cmsg.error_desc().c_str());
				break;
			}

			if (network_id_ != hello.network_id()) {
				cmsg.set_error_code(protocol::ERRCODE_INVALID_PARAMETER);
				cmsg.set_error_desc(utils::String::Format("Different network id, remote id(" FMT_I64 ") is not equal to the local id(" FMT_I64 ")",
					hello.network_id(), network_id_));
				LOG_ERROR("Failed to process the peer hello message.%s", cmsg.error_desc().c_str());
				break;
			}

			if (CheckSameChain(General::GetSelfChainId(), hello.chain_id())) {
				cmsg.set_error_code(protocol::ERRCODE_INVALID_PARAMETER);
				cmsg.set_error_desc(utils::String::Format("The peer connection is broken because it connects itself"));
				LOG_ERROR("Failed to process the peer hello message.%s", cmsg.error_desc().c_str());
				break;
			}

			cmsg.set_error_code(protocol::ERRCODE_SUCCESS);
			std::string error_desc_temp = utils::String::Format("Received a message channel hello message from ip(%s), and sent the response result(%d:%s)",
				conn->GetPeerAddress().ToIpPort().c_str(), ignore_ec.value(), ignore_ec.message().c_str());
			cmsg.set_error_desc(error_desc_temp.c_str());
			LOG_INFO("Received a message channel hello message from ip(%s), and sent the response result(%d:%s)", conn->GetPeerAddress().ToIpPort().c_str(),
				ignore_ec.value(), ignore_ec.message().c_str());

		} while (false);

		conn->SendResponse(message, cmsg.SerializeAsString(), ignore_ec);
		return cmsg.error_code() == 0;
	}

	bool MessageChannel::ChainExist(int64_t peer_id, int64_t chain_id) {
		bool exist = false;
		for (ConnectionMap::iterator iter = connections_.begin(); iter != connections_.end(); iter++) {
			MessageChannelPeer *peer = (MessageChannelPeer *)iter->second;
			bool same_node = CheckSameChain(chain_id, peer->GetChainId());
			if (same_node && peer->GetId() != peer_id) {
				exist = true;
				break;
			}
		}
		return exist;
	}

	bool MessageChannel::CheckSameChain(int64_t local_chain_id, int64_t target_chain_id){
		return  (local_chain_id == target_chain_id);
	}

	int64_t MessageChannel::GetChainIdFromConn(int64_t conn_id){
		MessageChannelPeer *peer = (MessageChannelPeer *)GetConnection(conn_id);
		if (!peer){
			LOG_ERROR("The target peer cannot be found in conection list.");
			return -1;
		}

		return peer->GetChainId();
	}

	bool MessageChannel::OnMessageChannel(protocol::WsMessage &message, int64_t conn_id){
		protocol::MessageChannel message_channel;
		message_channel.ParseFromString(message.data());
		Notify(message_channel);
		return true;
	}


	void MessageChannel::BroadcastMsg(int64_t type, const std::string &data) {
		utils::MutexGuard guard(conns_list_lock_);

		for (ConnectionMap::iterator iter = connections_.begin();
			iter != connections_.end();
			iter++) {
			std::error_code ec;
			iter->second->SendRequest(type, data, ec);
		}
	}

	void MessageChannel::BroadcastChainTxMsg(const protocol::MessageChannel& tx_msg) {
		utils::MutexGuard guard(conns_list_lock_);

		for (auto iter = connections_.begin(); iter != connections_.end(); iter++) {
			MessageChannelPeer *messageChannel = (MessageChannelPeer *)iter->second;
			if (messageChannel->GetChainId() == tx_msg.target_chain_id()) {
				std::error_code ec;
				std::string str = tx_msg.SerializeAsString();
				messageChannel->SendRequest(protocol::MESSAGE_CHANNEL_NODE_PACKAGE, str, ec);
			}
		}
	}

	void MessageChannel::GetModuleStatus(Json::Value &data) {
		data["name"] = "message_channel";
		data["listen_port"] = GetListenPort();
		Json::Value &peers = data["clients"];
		int32_t active_size = 0;
		utils::MutexGuard guard(conns_list_lock_);
		for (auto &item : connections_) {
			item.second->ToJson(peers[peers.size()]);
		}
	}

	bool MessageChannel::OnConnectOpen(Connection *conn) {
		const MessageChannelConfigure &message_channel_configure = Configure::Instance().message_channel_configure_;

		if (!conn->InBound()) {
			MessageChannelPeer *peer = (MessageChannelPeer *)conn;
			utils::InetAddress address(message_channel_configure.listen_address_);
			peer->SendHello(address.ToIp(), message_channel_configure.network_id_, last_ec_);
		}
		return true;

	}


	void MessageChannel::OnDisconnect(Connection *conn) {
		MessageChannelPeer *peer = (MessageChannelPeer *)conn;
		LOG_INFO("The MessageChannelPeer has been disconnected, node address is (%s)", peer->GetPeerAddress().ToIpPort().c_str());
	}

	Connection *MessageChannel::CreateConnectObject(server *server_h, client *client_,
		tls_server *tls_server_h, tls_client *tls_client_h,
		connection_hdl con, const std::string &uri, int64_t id) {
		return new MessageChannelPeer(server_h, client_, tls_server_h, tls_client_h, con, uri, id);
	}

	bool MessageChannel::ConnectToMessageChannel() {
		const MessageChannelConfigure &message_channel_configure = Configure::Instance().message_channel_configure_;
		do {
			utils::StringList::const_iterator itor;
			itor = message_channel_configure.known_message_channel_list_.begin();
			utils::MutexGuard guard(conns_list_lock_);
			while (itor != message_channel_configure.known_message_channel_list_.end()){
				std::string address = *itor++;
				std::string uri = utils::String::Format("%s://%s", ssl_parameter_.enable_ ? "wss" : "ws", address.c_str());
				Connect(uri);
			}
			return true;
		} while (false);

		return false;
	}

	bool MessageChannel::SendRequest(int64_t id, int64_t type, const std::string &data){
		return true;
	}

	bool MessageChannel::ReceiveMsg(int64_t type, const std::string &data, int64_t id){
		return true;
	}

	void MessageChannel::OnTimer(int64_t current_time){

		if (current_time < 10 * utils::MICRO_UNITS_PER_SEC + last_uptate_time_)
		{
			return;
		}

		const MessageChannelConfigure &message_channel_configure = Configure::Instance().message_channel_configure_;
		size_t con_size = 0;
		utils::MutexGuard guard(conns_list_lock_);
		ConnectionMap::const_iterator iter;
		utils::StringList listTempIptoPort;
		iter = connections_.begin();
		while (iter != connections_.end()) {
			listTempIptoPort.push_back(iter->second->GetPeerAddress().ToIpPort().c_str());
			iter++;
		}

		utils::StringList::const_iterator itor;
		itor = message_channel_configure.known_message_channel_list_.begin();
		while (itor != message_channel_configure.known_message_channel_list_.end()){
			std::string address = *itor++;
			utils::StringList::const_iterator listTempIptoPortiter;
			listTempIptoPortiter = std::find(listTempIptoPort.begin(), listTempIptoPort.end(), address.c_str());

			if (listTempIptoPortiter != listTempIptoPort.end()){

			}
			else{
				std::string uri = utils::String::Format("%s://%s", ssl_parameter_.enable_ ? "wss" : "ws", address.c_str());
				Connect(uri);
			}
		}
		last_uptate_time_ = current_time;
	}

	bool MessageChannel::OnVerifyCallback(bool preverified, asio::ssl::verify_context& ctx) {
		return true;
	}

	//croos message hander
	void MessageChannel::RegisterMessageChannelConsumer(MessageChannelConsumer *msg_consumer, int64_t msg_type){
		utils::MutexGuard guard(message_channel_consumer_lock_);

		if (message_channel_consumer_.empty()){
			message_channel_consumer_.insert(make_pair(msg_type, msg_consumer));
		}

		bool is_exist = false;
		map<int64_t, MessageChannelConsumer*>::iterator it = message_channel_consumer_.lower_bound(msg_type);
		while (it != message_channel_consumer_.upper_bound(msg_type)){
			if (it->second == msg_consumer){
				is_exist = true;
			}
			it++;
		}

		if (!is_exist){
			message_channel_consumer_.insert(make_pair(msg_type, msg_consumer));
		}
	}

	void MessageChannel::UnregisterMessageChannelConsumer(MessageChannelConsumer *msg_consumer, int64_t msg_type){
		utils::MutexGuard guard(message_channel_consumer_lock_);
		if (message_channel_consumer_.empty()){
			return;
		}

		map<int64_t, MessageChannelConsumer*>::iterator it = message_channel_consumer_.lower_bound(msg_type);
		while (it != message_channel_consumer_.upper_bound(msg_type)){
			if (it->second == msg_consumer){
				message_channel_consumer_.erase(it);
				break;
			}
			it++;
		}
	}

	void MessageChannel::Notify(const protocol::MessageChannel &message_channel){
		utils::MutexGuard guard(message_channel_consumer_lock_);
		map<int64_t, MessageChannelConsumer*>::iterator it = message_channel_consumer_.lower_bound(message_channel.msg_type());
		for (; it != message_channel_consumer_.upper_bound(message_channel.msg_type()); it++){
			it->second->HandleMessageChannelConsumer(message_channel);
		}
	}


}

