#include "message_channel_manager.h"

namespace bumo {

	MessageChannelManager::MessageChannelManager(server *server_h, client *client_h, tls_server *tls_server_h, tls_client *tls_client_h, connection_hdl con, const std::string &uri, int64_t id) :
		Connection(server_h, client_h, tls_server_h, tls_client_h, con, uri, id) {
	}


	MessageChannelManager::~MessageChannelManager(){
	}



	MessageChannelServer::MessageChannelServer() : Network(SslParameter()) {
		connect_interval_ = 120 * utils::MICRO_UNITS_PER_SEC;
		last_connect_time_ = 0;

		/*request_methods_[protocol::MESSAGE_CHANNEL_CREATE_CHILD_CHAIN] = std::bind(&MessageChannelServer::OnCrateChildChain, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::MESSAGE_CHANNEL_MAIN_MIX] = std::bind(&MessageChannelServer::OnMainChainMix, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::CHAIN_SUBMITTRANSACTION] = std::bind(&MessageChannelServer::OnSubmitTransaction, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::CHAIN_SUBSCRIBE_TX] = std::bind(&MessageChannelServer::OnSubscribeTx, this, std::placeholders::_1, std::placeholders::_2);*/
		thread_ptr_ = NULL;
	}

	MessageChannelServer::~MessageChannelServer() {
		if (thread_ptr_){
			delete thread_ptr_;
		}
	}

	bool MessageChannelServer::Initialize(MessageChannelConfigure & message_channel_configure) {
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("messageChannel")) {
			return false;
		}

		StatusModule::RegisterModule(this);
		LOG_INFO("Initialized message channel server successfully");
		return true;
	}

	bool MessageChannelServer::Exit() {
		Stop();
		thread_ptr_->JoinWithStop();
		return true;
	}

	void MessageChannelServer::Run(utils::Thread *thread) {
		Start(bumo::Configure::Instance().message_channel_configure_.listen_address_);
	}


	bool MessageChannelServer::OnCrateChildChain(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	bool MessageChannelServer::OnMainChainMix(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	bool MessageChannelServer::OnChildChainMix(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}
	bool MessageChannelServer::OnDeposit(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	bool MessageChannelServer::OnWithdrawal(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	bool MessageChannelServer::OnFastWithdrawal(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	bool MessageChannelServer::OnSubmitChildHead(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	bool MessageChannelServer::OnChallengeWithdrawal(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	bool MessageChannelServer::OnChallengeChildHead(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	bool MessageChannelServer::OnChildChainGeneses(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	bool MessageChannelServer::OnSubmitTransaction(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	bool MessageChannelServer::OnSubscribeTx(protocol::MessageChannel &message, int64_t conn_id){
		return true;
	}

	void MessageChannelServer::BroadcastMsg(int64_t type, const std::string &data) {
		utils::MutexGuard guard(conns_list_lock_);

		for (ConnectionMap::iterator iter = connections_.begin();
			iter != connections_.end();
			iter++) {
			std::error_code ec;
			iter->second->SendRequest(type, data, ec);
		}
	}

	void MessageChannelServer::BroadcastChainTxMsg(const protocol::TransactionEnvStore& tx_msg) {
		utils::MutexGuard guard(conns_list_lock_);

	/*	for (auto iter = connections_.begin(); iter != connections_.end(); iter++) {
			WsPeer *peer = (WsPeer *)iter->second;
			if (peer->Filter(tx_msg)) {
				std::error_code ec;
				std::string str = tx_msg.SerializeAsString();
				peer->SendRequest(protocol::CHAIN_TX_ENV_STORE, str, ec);
			}
		}*/
	}

	void MessageChannelServer::GetModuleStatus(Json::Value &data) {
		data["name"] = "message_channel_server";
		data["listen_port"] = GetListenPort();
		Json::Value &peers = data["clients"];
		int32_t active_size = 0;
		utils::MutexGuard guard(conns_list_lock_);
		for (auto &item : connections_) {
			item.second->ToJson(peers[peers.size()]);
		}
	}

	Connection *MessageChannelServer::CreateConnectObject(server *server_h, client *client_,
		tls_server *tls_server_h, tls_client *tls_client_h,
		connection_hdl con, const std::string &uri, int64_t id) {
		return new MessageChannelManager(server_h, client_, tls_server_h, tls_client_h, con, uri, id);
	}
}