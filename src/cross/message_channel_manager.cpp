#include <utils/headers.h>
#include <common/general.h>
#include <main/configure.h>
#include <proto/cpp/monitor.pb.h>
#include <overlay/peer_manager.h>
#include <glue/glue_manager.h>
#include <ledger/ledger_manager.h>
#include <monitor/monitor.h>

#include "message_channel_manager.h"

namespace bumo {

	MessageChannelPeer::MessageChannelPeer(server *server_h, client *client_h, tls_server *tls_server_h, tls_client *tls_client_h, connection_hdl con, const std::string &uri, int64_t id) :
		Connection(server_h, client_h, tls_server_h, tls_client_h, con, uri, id) {
		active_time_ = 0;
		delay_ = 0;
		peer_listen_port_ = 0;
		chain_id_ = 0;
		
	}


	MessageChannelPeer::~MessageChannelPeer(){
	}


	utils::InetAddress MessageChannelPeer::GetRemoteAddress() const {
		utils::InetAddress address = GetPeerAddress();
		if (InBound()) {
			address.SetPort((uint16_t)peer_listen_port_);
		}
		return address;
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

		peer_listen_port_ = hello.listening_port();
		peer_node_address_ = hello.node_address();
		chain_id_ = hello.chain_id();
	}

	void MessageChannelPeer::SetActiveTime(int64_t current_time) {
		active_time_ = current_time;
	}

	bool MessageChannelPeer::SendHello(int32_t listen_port, const std::string &node_address, const int64_t &network_id, std::error_code &ec) {
		protocol::MessageChannelHello hello;


		hello.set_listening_port(listen_port);
		hello.set_node_address(node_address);
		hello.set_network_id(network_id);
		hello.set_chain_id(General::GetSelfChainId());
		return SendRequest(protocol::MESSAGE_CHANNEL_HELLO, hello.SerializeAsString(), ec);
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



	bool MessageChannelPeer::Set(const protocol::ChainSubscribeTx &sub) {
		if (sub.address_size() > 100) {
			LOG_ERROR("Failed  to subscribe address, size large than 100");
			return false;
		}

		tx_filter_address_.clear();
		for (int32_t i = 0; i < sub.address_size(); i++) {
			if (!PublicKey::IsAddressValid(sub.address(i))) {
				LOG_ERROR("Failed to subscribe address, address(%s) not valid", sub.address(i).c_str());
				return false;
			}
			tx_filter_address_.insert(sub.address(i));
		}

		return true;
	}

	bool MessageChannelPeer::Filter(const protocol::TransactionEnvStore &tx_msg) {
		return false;
	}





	MessageChannel::MessageChannel() : Network(SslParameter()) {
		connect_interval_ = 120 * utils::MICRO_UNITS_PER_SEC;
		last_connect_time_ = 0;
		last_uptate_time_ = utils::Timestamp::HighResolution();

		request_methods_[protocol::MESSAGE_CHANNEL_HELLO] = std::bind(&MessageChannel::OnHello, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN] = std::bind(&MessageChannel::OnCrateChildChain, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_MAIN_MIX] = std::bind(&MessageChannel::OnMainChainMix, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_CHILD_MIX] = std::bind(&MessageChannel::OnChildChainMix, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_DEPOSIT] = std::bind(&MessageChannel::OnDeposit, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_WITHDRAWAL] = std::bind(&MessageChannel::OnWithdrawal, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_FAST_WITHDRAWAL] = std::bind(&MessageChannel::OnFastWithdrawal, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_SUBMIT_HEAD] = std::bind(&MessageChannel::OnSubmitChildHead, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_CHALLENGE_WITHDRAWAL] = std::bind(&MessageChannel::OnChallengeWithdrawal, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_CHALLENGE_HEAD] = std::bind(&MessageChannel::OnChallengeChildHead, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_CHILD_GENESES] = std::bind(&MessageChannel::OnChildChainGeneses, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::CHAIN_SUBMITTRANSACTION] = std::bind(&MessageChannel::OnSubmitTransaction, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::CHAIN_SUBSCRIBE_TX] = std::bind(&MessageChannel::OnSubscribeTx, this, std::placeholders::_1, std::placeholders::_2);
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
		protocol::MessageChannelHelloResponse cmsg;
		std::error_code ignore_ec;

		utils::MutexGuard guard_(conns_list_lock_);
		Connection *conn = GetConnection(conn_id);
		
		if (conn) {
			cmsg.set_error_code(protocol::ERRCODE_SUCCESS);
			std::string error_desc_temp = utils::String::Format("Received a message channel hello message from ip(%s), and sent the response result(%d:%s)", 
				conn->GetPeerAddress().ToIpPort().c_str(),ignore_ec.value(), ignore_ec.message().c_str());
			cmsg.set_error_desc(error_desc_temp.c_str());
			conn->SendResponse(message, cmsg.SerializeAsString(), ignore_ec);
			LOG_INFO("Received a message channel hello message from ip(%s), and sent the response result(%d:%s)", conn->GetPeerAddress().ToIpPort().c_str(),
				ignore_ec.value(), ignore_ec.message().c_str());
		}
		return true;
	}

	bool MessageChannel::OnCrateChildChain(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}

	bool MessageChannel::OnMainChainMix(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}

	bool MessageChannel::OnChildChainMix(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}
	bool MessageChannel::OnDeposit(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}

	bool MessageChannel::OnWithdrawal(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}

	bool MessageChannel::OnFastWithdrawal(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}

	bool MessageChannel::OnSubmitChildHead(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}

	bool MessageChannel::OnChallengeWithdrawal(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}

	bool MessageChannel::OnChallengeChildHead(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}

	bool MessageChannel::OnChildChainGeneses(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}

	bool MessageChannel::OnSubmitTransaction(protocol::WsMessage &message, int64_t conn_id){
		return true;
	}

	bool MessageChannel::OnSubscribeTx(protocol::WsMessage &message, int64_t conn_id){
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

	void MessageChannel::BroadcastChainTxMsg(const protocol::TransactionEnvStore& tx_msg) {
		utils::MutexGuard guard(conns_list_lock_);

		for (auto iter = connections_.begin(); iter != connections_.end(); iter++) {
			MessageChannelPeer *messageChannel = (MessageChannelPeer *)iter->second;
			if (messageChannel->Filter(tx_msg)) {
				std::error_code ec;
				std::string str = tx_msg.SerializeAsString();
				messageChannel->SendRequest(protocol::CHAIN_TX_ENV_STORE, str, ec);
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
		size_t total_connection = message_channel_configure.max_connection_;
		if (connections_.size() < total_connection) {
			if (!conn->InBound()) {
				MessageChannelPeer *peer = (MessageChannelPeer *)conn;
				utils::InetAddress address(message_channel_configure.listen_address_);
				peer->SendHello(address.GetPort(), address.ToIp(), message_channel_configure.network_id_, last_ec_);
			}
			return true;
		}
		else{
			LOG_ERROR("Failed to open a connection, because it exceeds the threshold(" FMT_SIZE ")", total_connection);
			return false;
		}
	}


	void MessageChannel::OnDisconnect(Connection *conn) {
		MessageChannelPeer *peer = (MessageChannelPeer *)conn;
		std::string uri = utils::String::Format("%s://%s", ssl_parameter_.enable_ ? "wss" : "ws", peer->GetPeerAddress().ToIpPort().c_str());
		Connect(uri);
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

		if (current_time<10 *utils::MICRO_UNITS_PER_SEC + last_uptate_time_)
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


}