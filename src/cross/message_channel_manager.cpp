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

#include <utils/headers.h>
#include <utils/random.h>
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

	std::string MessageChannelPeer::GetRoundString() const {
		return round_string_;
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

	void MessageChannelPeer::SetRoundString(const std::string &round_string) {
		round_string_ = round_string;
	}

	bool MessageChannelPeer::SendHello(const std::string &node_address, const int64_t &network_id, std::error_code &ec) {
		protocol::MessageChannelHello hello;

		hello.set_node_address(node_address);
		hello.set_network_id(network_id);
		hello.set_chain_id(General::GetSelfChainId());
		std::string round_str;
		utils::GetStrongRandBytes(round_str);
		hello.set_round_string(round_str);
		SetRoundString(round_str);
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

		response_methods_[protocol::MESSAGE_CHANNEL_NODE_HELLO] = std::bind(&MessageChannel::OnHelloResponse, this, std::placeholders::_1, std::placeholders::_2);
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

		const P2pConfigure &p2p_configure = Configure::Instance().p2p_configure_;
		network_id_ = p2p_configure.network_id_;

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

			if (network_id_ != hello.network_id()) {
				cmsg.set_error_code(protocol::ERRCODE_INVALID_PARAMETER);
				cmsg.set_error_desc(utils::String::Format("Different network id, remote id(" FMT_I64 ") is not equal to the local id(" FMT_I64 ")",
					hello.network_id(), network_id_));
				LOG_ERROR("Failed to process the peer hello message.%s", cmsg.error_desc().c_str());
				break;
			}

			if (General::GetSelfChainId() == hello.chain_id()) {
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

			LOG_INFO("Received a hello message, peer(%s) is active", conn->GetPeerAddress().ToIpPort().c_str());
			peer->SetActiveTime(utils::Timestamp::HighResolution());
			//append signature
			std::string private_key = bumo::Configure::Instance().ledger_configure_.validation_privatekey_;
			PrivateKey pkey(private_key);
			if (!pkey.IsValid()){
				cmsg.set_error_code(protocol::ERRCODE_INVALID_PRIKEY);
				cmsg.set_error_desc(utils::String::Format("The peer connection is broken because invalid prikey"));
				LOG_ERROR("Failed to process the peer hello message.%s", cmsg.error_desc().c_str());
				break;
			}
			std::string sign = pkey.Sign(hello.round_string());
			protocol::Signature *signpro = cmsg.mutable_round_signature();
			signpro->set_sign_data(sign);
			signpro->set_public_key(pkey.GetEncPublicKey());

			if (peer->InBound()) {
				const MessageChannelConfigure &message_channel_configure = Configure::Instance().message_channel_configure_;
				const P2pConfigure &p2p_configure = Configure::Instance().p2p_configure_;
				utils::InetAddress address(message_channel_configure.listen_address_);
				peer->SendHello(address.ToIp(), p2p_configure.network_id_, last_ec_);
			}


		} while (false);

		conn->SendResponse(message, cmsg.SerializeAsString(), ignore_ec);
		return cmsg.error_code() == 0;
	}

	bool MessageChannel::OnHelloResponse(protocol::WsMessage &message, int64_t conn_id){
		utils::MutexGuard guard(conns_list_lock_);
		MessageChannelPeer *peer = (MessageChannelPeer *)GetConnection(conn_id);
		if (!peer) {
			return true;
		}

		protocol::MessageChannelHelloResponse env;
		env.ParseFromString(message.data());
		if (env.error_code() != 0) {
			LOG_ERROR("Failed to response the MessageChannelPeer hello message.MessageChannelPeer reponse error code(%d), desc(%s)", env.error_code(), env.error_desc().c_str());
			return false;
		}
		const protocol::Signature &sig = env.round_signature();
		//get local key public key
		std::string private_key = bumo::Configure::Instance().ledger_configure_.validation_privatekey_;
		PrivateKey pkey(private_key);
		std::string error_str;
		if (!pkey.IsValid()){
			peer->Close("OnHelloResponse get privatekey failed");
			LOG_ERROR("OnHelloResponse get privatekey failed.");
			return false;
		}
		if (!PublicKey::Verify(peer->GetRoundString(), sig.sign_data(), pkey.GetEncPublicKey())) {
			error_str = utils::String::Format(" (ip:%s)(address:%s) is not Main-Child match chain", 
				peer->GetPeerAddress().ToIpPort().c_str(), peer->GetPeerNodeAddress().c_str());
			peer->Close(error_str);
			LOG_ERROR("errorstr:%s",error_str.c_str());
			return false;
		}
		return true;
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

	void MessageChannel::MessageChannelProducer(const protocol::MessageChannel& message_channel) {
		if (message_channel.target_chain_id() == General::GetSelfChainId()){
			Notify(message_channel);
		}

		utils::MutexGuard guard(conns_list_lock_);
		for (auto iter = connections_.begin(); iter != connections_.end(); iter++) {
			MessageChannelPeer *messageChannel = (MessageChannelPeer *)iter->second;
			if (messageChannel->GetChainId() == message_channel.target_chain_id()) {
				std::error_code ec;
				std::string str = message_channel.SerializeAsString();
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
		const P2pConfigure &p2p_configure = Configure::Instance().p2p_configure_;

		if (!conn->InBound()) {
			MessageChannelPeer *peer = (MessageChannelPeer *)conn;
			utils::InetAddress address(message_channel_configure.listen_address_);
			peer->SendHello(address.ToIp(), p2p_configure.network_id_, last_ec_);
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

	void MessageChannel::ProcessMessageChannelDisconnect(){
		if (General::GetSelfChainId() == General::MAIN_CHAIN_ID){
			return;
		}

		const MessageChannelConfigure &message_channel_configure = Configure::Instance().message_channel_configure_;
		utils::MutexGuard guard(conns_list_lock_);
		ConnectionMap::const_iterator iter;
		utils::StringList listTempIptoPort;
		iter = connections_.begin();
		while (iter != connections_.end()) {
			listTempIptoPort.push_back(iter->second->GetPeerAddress().ToIpPort().c_str());
			iter++;
		}

		std::string address = message_channel_configure.target_message_channel_.ToIpPort().c_str();

		utils::StringList::const_iterator listTempIptoPortiter;
		listTempIptoPortiter = std::find(listTempIptoPort.begin(), listTempIptoPort.end(), address.c_str());

		if (listTempIptoPortiter == listTempIptoPort.end()){
			std::string uri = utils::String::Format("%s://%s", ssl_parameter_.enable_ ? "wss" : "ws", address.c_str());
			Connect(uri);
		}
	}

	void MessageChannel::OnTimer(int64_t current_time){

		if (current_time > 10 * utils::MICRO_UNITS_PER_SEC + last_uptate_time_){

			ProcessMessageChannelDisconnect();
			last_uptate_time_ = current_time;
		}

	}

	bool MessageChannel::OnVerifyCallback(bool preverified, asio::ssl::verify_context& ctx) {
		return true;
	}

	//croos message hander
	void MessageChannel::RegisterMessageChannelConsumer(IMessageChannelConsumer *msg_consumer, int64_t msg_type){
		utils::MutexGuard guard(message_channel_consumer_lock_);

		if (message_channel_consumer_.empty()){
			message_channel_consumer_.insert(make_pair(msg_type, msg_consumer));
		}

		bool is_exist = false;
		map<int64_t, IMessageChannelConsumer*>::iterator it = message_channel_consumer_.lower_bound(msg_type);
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

	void MessageChannel::UnregisterMessageChannelConsumer(IMessageChannelConsumer *msg_consumer, int64_t msg_type){
		utils::MutexGuard guard(message_channel_consumer_lock_);
		if (message_channel_consumer_.empty()){
			return;
		}

		map<int64_t, IMessageChannelConsumer*>::iterator it = message_channel_consumer_.lower_bound(msg_type);
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
		map<int64_t, IMessageChannelConsumer*>::iterator it = message_channel_consumer_.lower_bound(message_channel.msg_type());
		for (; it != message_channel_consumer_.upper_bound(message_channel.msg_type()); it++){
			it->second->HandleMessageChannelConsumer(message_channel);
		}
	}


}

