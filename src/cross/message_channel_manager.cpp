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

	MessageChannelManager::MessageChannelManager(server *server_h, client *client_h, tls_server *tls_server_h, tls_client *tls_client_h, connection_hdl con, const std::string &uri, int64_t id) :
		Connection(server_h, client_h, tls_server_h, tls_client_h, con, uri, id) {
	}


	MessageChannelManager::~MessageChannelManager(){
	}

	bool MessageChannelManager::Set(const protocol::ChainSubscribeTx &sub) {
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

	bool MessageChannelManager::Filter(const protocol::TransactionEnvStore &tx_msg) {
		return false;
	}





	MessageChannel::MessageChannel() : Network(SslParameter()) {
		connect_interval_ = 120 * utils::MICRO_UNITS_PER_SEC;
		last_connect_time_ = 0;

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
		LOG_INFO("Initialized message channel server successfully");
		return true;
	}

	bool MessageChannel::Exit() {
		Stop();
		thread_ptr_->JoinWithStop();
		return true;
	}

	void MessageChannel::Run(utils::Thread *thread) {
		Start(bumo::Configure::Instance().message_channel_configure_.listen_address_);
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
				MessageChannelManager *messageChannel = (MessageChannelManager *)iter->second;
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

	Connection *MessageChannel::CreateConnectObject(server *server_h, client *client_,
		tls_server *tls_server_h, tls_client *tls_client_h,
		connection_hdl con, const std::string &uri, int64_t id) {
		return new MessageChannelManager(server_h, client_, tls_server_h, tls_client_h, con, uri, id);
	}
}