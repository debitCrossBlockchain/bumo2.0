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
#include <common/general.h>
#include <main/configure.h>
#include <proto/cpp/monitor.pb.h>
#include <overlay/peer_manager.h>
#include <glue/glue_manager.h>
#include <ledger/ledger_manager.h>
#include <monitor/monitor.h>

#include "websocket_server.h"

namespace bumo {
	WebSocketServer::WebSocketServer() : Network(SslParameter()) {
		connect_interval_ = 120 * utils::MICRO_UNITS_PER_SEC;
		last_connect_time_ = 0;

		request_methods_[protocol::CHAIN_HELLO] = std::bind(&WebSocketServer::OnChainHello, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::CHAIN_PEER_MESSAGE] = std::bind(&WebSocketServer::OnChainPeerMessage, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::CHAIN_SUBMITTRANSACTION] = std::bind(&WebSocketServer::OnSubmitTransaction, this, std::placeholders::_1, std::placeholders::_2);
		request_methods_[protocol::CHAIN_SUBSCRIPTION] = std::bind(&WebSocketServer::OnSubscription, this, std::placeholders::_1, std::placeholders::_2);
		thread_ptr_ = NULL;
	}

	WebSocketServer::~WebSocketServer() {
		if (thread_ptr_){
			delete thread_ptr_;
		} 
	}

	bool WebSocketServer::Initialize(WsServerConfigure &ws_server_configure) {
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("websocket")) {
			return false;
		}

		StatusModule::RegisterModule(this);
		LOG_INFO("Websocket server initialized");
		return true;
	}

	bool WebSocketServer::Exit() {
		Stop();
		thread_ptr_->JoinWithStop();
		return true;
	}

	void WebSocketServer::Run(utils::Thread *thread) {
		Start(bumo::Configure::Instance().wsserver_configure_.listen_address_);
	}

	bool WebSocketServer::OnChainHello(protocol::WsMessage &message, int64_t conn_id) {
		protocol::ChainStatus cmsg;
		cmsg.set_bumo_version(General::BUMO_VERSION);
		cmsg.set_monitor_version(General::MONITOR_VERSION);
		cmsg.set_ledger_version(General::LEDGER_VERSION);
		cmsg.set_self_addr(PeerManager::Instance().GetPeerNodeAddress());
		cmsg.set_timestamp(utils::Timestamp::HighResolution());
		std::error_code ignore_ec;

		utils::MutexGuard guard_(conns_list_lock_);
		Connection *conn = GetConnection(conn_id);
		if (conn) {
			conn->SendResponse(message, cmsg.SerializeAsString(), ignore_ec);
			LOG_INFO("Recv chain hello from ip(%s), send response result(%d:%s)", conn->GetPeerAddress().ToIpPort().c_str(),
				ignore_ec.value(), ignore_ec.message().c_str());
		}
		return true;
	}

	bool WebSocketServer::OnChainPeerMessage(protocol::WsMessage &message, int64_t conn_id) {
		// send peer
		utils::MutexGuard guard_(conns_list_lock_);
		Connection *conn = GetConnection(conn_id);
		if (!conn) {
			return false;
		}

		LOG_INFO("Recv chain peer message from ip(%s)", conn->GetPeerAddress().ToIpPort().c_str());
		protocol::ChainPeerMessage cpm;
		if (!cpm.ParseFromString(message.data())) {
			LOG_ERROR("ChainPeerMessage FromString fail");
			return true;
		}

		//bubi::PeerManager::Instance().BroadcastPayLoad(cpm);
		return true;
	}

	bool WebSocketServer::FilterByAddress(std::string address, protocol::TransactionEnvStore& txMsg){
		if (address.empty()){
			return true;
		}

		auto trans = txMsg.transaction_env().transaction();
		if (trans.source_address() == address){
			return true;
		}

		unsigned len = trans.operations().size();
		for (unsigned i = 0; i < len; i++)
		{
			auto ope = trans.mutable_operations(i);
			if (ope->source_address() == address){
				return true;
			}

			if (ope->type() == protocol::Operation_Type_CREATE_ACCOUNT){
				if (ope->create_account().dest_address() == address){
					return true;
				}
			}
			else if (ope->type() == protocol::Operation_Type_PAY_COIN){
				if (ope->payment().dest_address() == address){
					return true;
				}
			}
			else if (ope->type() == protocol::Operation_Type_PAYMENT){
				if (ope->pay_coin().dest_address() == address){
					return true;
				}
			}
		}

		return false;
	}

	void WebSocketServer::BroadcastMsg(int64_t type, const std::string &data) {
		utils::MutexGuard guard(conns_list_lock_);

		for (ConnectionMap::iterator iter = connections_.begin();
			iter != connections_.end();
			iter++) {
			std::error_code ec;
			iter->second->SendRequest(type, data, ec);
		}
	}

	void WebSocketServer::BroadcastChainTxMsg(protocol::TransactionEnvStore& txMsg) {
		std::string str = txMsg.SerializeAsString();
		bumo::WebSocketServer::Instance().BroadcastMsg(protocol::CHAIN_TX_STATUS, str);
		utils::MutexGuard guard(conns_list_lock_);

		for (auto iter = connections_.begin(); iter != connections_.end(); iter++) {
			if (subscriptions_.find(iter->first) == subscriptions_.end()){
				std::error_code ec;
				iter->second->SendRequest(protocol::CHAIN_TX_ENV_STORE, str, ec);
			}
			else{
				if(FilterByAddress(subscriptions_[iter->first], txMsg)){
					std::error_code ec;
					iter->second->SendRequest(protocol::CHAIN_TX_ENV_STORE, str, ec);
				}
			}
		}
	}

	bool WebSocketServer::OnSubmitTransaction(protocol::WsMessage &message, int64_t conn_id) {
		utils::MutexGuard guard_(conns_list_lock_);
		Connection *conn = GetConnection(conn_id);
		if (!conn) {
			return false;
		}

		Result result;
		protocol::TransactionEnv tran_env;
		do {
			if (!tran_env.ParseFromString(message.data())) {
				LOG_ERROR("Parse submit transaction string fail, ip(%s)", conn->GetPeerAddress().ToIpPort().c_str());
				result.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result.set_desc("Parse the transaction failed");
				break;
			}
			Json::Value real_json;
			real_json = Proto2Json(tran_env);
			printf("%s",real_json.toStyledString().c_str());
			std::string content = tran_env.transaction().SerializeAsString();

			TransactionFrm::pointer ptr = std::make_shared<TransactionFrm>(tran_env);
			GlueManager::Instance().OnTransaction(ptr, result);
			PeerManager::Instance().Broadcast(protocol::OVERLAY_MSGTYPE_TRANSACTION, tran_env.SerializeAsString());
		
		} while (false);

		//notice WebSocketServer Tx status
		std::string hash = HashWrapper::Crypto(tran_env.transaction().SerializeAsString());
		protocol::ChainTxStatus cts;
		cts.set_tx_hash(utils::String::BinToHexString(hash));
		cts.set_error_code((protocol::ERRORCODE)result.code());
		cts.set_source_address(tran_env.transaction().source_address());
		cts.set_status(result.code() == protocol::ERRCODE_SUCCESS ? protocol::ChainTxStatus_TxStatus_CONFIRMED : protocol::ChainTxStatus_TxStatus_FAILURE);
		cts.set_error_desc(result.desc());
		cts.set_timestamp(utils::Timestamp::Now().timestamp());
		std::string str = cts.SerializeAsString();
			
		BroadcastMsg(protocol::CHAIN_TX_STATUS, str);
		
		return true;
	}

	bool WebSocketServer::OnSubscription(protocol::WsMessage &message, int64_t conn_id){
		utils::MutexGuard guard_(conns_list_lock_);
		Connection *conn = GetConnection(conn_id);
		if (!conn) {
			return false;
		}

		LOG_INFO("Recv chain peer message from ip(%s)", conn->GetPeerAddress().ToIpPort().c_str());
		protocol::ChainSubscription subs;
		if (!subs.ParseFromString(message.data())) {
			LOG_ERROR("ChainPeerMessage FromString fail");
			return true;
		}

		subscriptions_[conn_id] = subs.address();
		return true;
	}

	void WebSocketServer::GetModuleStatus(Json::Value &data) {
		data["name"] = "websocket_server";
		data["listen_port"] = GetListenPort();
		Json::Value &peers = data["clients"];
		int32_t active_size = 0;
		utils::MutexGuard guard(conns_list_lock_);
		for (auto &item : connections_) {
			item.second->ToJson(peers[peers.size()]);
		}
	}
}