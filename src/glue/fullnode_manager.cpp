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
#include <proto/cpp/overlay.pb.h>
#include <proto/cpp/chain.pb.h>
#include <ledger/ledger_manager.h>
#include <overlay/peer_manager.h>
#include <common/network.h>

#include "fullnode.h"
#include "fullnode_manager.h"
#include "glue_manager.h"


namespace bumo {
	FullNodeManager::FullNodeManager() :
		last_ledger_seq_(0),
		fullnode_check_timer_(0),
		priv_key_(SIGNTYPE_CFCASM2),
		local_address_(""),
		Network(SslParameter()) {
		thread_ptr_ = NULL;
	}

	FullNodeManager::~FullNodeManager() {}

	bool FullNodeManager::Initialize() {
		if (!priv_key_.From(Configure::Instance().p2p_configure_.node_private_key_)) {
			LOG_ERROR("Initialize node private key failed");
			return false;
		}
		local_address_ = priv_key_.GetEncAddress();

		StatusModule::RegisterModule(this);
		TimerNotify::RegisterModule(this);
		return true;
	}

	void FullNodeManager::GetModuleStatus(Json::Value &data) {
		return;
	}

	void FullNodeManager::OnTimer(int64_t current_time){
		return;
	}

	void FullNodeManager::OnSlowTimer(int64_t current_time){
		return;
	}

	void FullNodeManager::Run(utils::Thread *this_thread){
		return;
	}

	FullNodePointer FullNodeManager::get(std::string& key) {
		auto it = full_node_info_.find(key);
		if (it != full_node_info_.end()) {
			return it->second;
		}
		else {
			return nullptr;
		}
	}

	bool FullNodeManager::add(FullNode& fn) {
		FullNodePointer fp = std::make_shared<FullNode>(fn);
		return add(fp);
	}

	bool FullNodeManager::add(FullNodePointer fp) {
		if (full_node_info_.find(fp->getAddress()) != full_node_info_.end()) {
			LOG_INFO("Full node address %s already exist", fp->getAddress());
			return true;
		}
		try
		{
			full_node_info_.insert(std::make_pair(fp->getAddress(), fp));
		}
		catch (std::exception& e) {
			LOG_ERROR("Insert %s to full node info map exception, %s", fp->getAddress().c_str(), e.what());
			return false;
		}
		return true;
	}

	void FullNodeManager::remove(std::string& key) {
		auto it = full_node_info_.find(key);
		if (it != full_node_info_.end()) {
			full_node_info_.erase(it);
		}
		else {
			LOG_INFO("The full node address(%s) to remove not exist", key);
		}
		return;
	}

	bool FullNodeManager::isInspector(const std::string& addr) {
		int32_t size = sorted_full_nodes_.size();
		if (size < 2) return false;
		int32_t index_range = size < 1000 ? 1 : size / 1000;
		for (int32_t i = 0; i < index_range; ++i) {
			if (sorted_full_nodes_[i] == addr) {
				return true;
			}
		}
		return false;
	}

	bool FullNodeManager::isUnderInspection(const std::string& addr) {
		int32_t size = sorted_full_nodes_.size();
		if (size < 2) return false;
		int32_t index_range = size < 1000 ? 1 : size / 1000;
		for (int32_t i = 0; i < index_range; ++i) {
			if (sorted_full_nodes_[size - 1 - i] == addr) {
				return true;
			}
		}
		return false;
	} 

	bool FullNodeManager::getPeerAddr(const std::string& addr, std::string& peer) {
		int32_t size = sorted_full_nodes_.size();
		if (size < 2) return false;
		int32_t index_range = size < 1000 ? 1 : size / 1000;
		for (int32_t i = 0; i < index_range; ++i) {
			if (sorted_full_nodes_[i] == addr) {
				peer = sorted_full_nodes_[size - 1 - i];
				return true;
			}
			else if (sorted_full_nodes_[size - 1 - i] == addr){
				peer = sorted_full_nodes_[i];
				return true;
			} else continue;
		}
		return false;
	}

	bool FullNodeManager::sortFullNode(const std::string& blockHash){
		sorted_full_nodes_.clear();
		std::map<std::string, std::string> sorted_map;
		for (auto it = full_node_info_.begin(); it != full_node_info_.end(); ++it) {
			std::string key = HashWrapper::Crypto(HashWrapper::Crypto(it->first) + blockHash);
			sorted_map.insert(std::make_pair(key, it->first));
		}
		for (auto it = sorted_map.begin(); it != sorted_map.end(); ++it) {
			sorted_full_nodes_.push_back(it->second);
		}
		return true;
	}

	bool FullNodeManager::verifyCheckAuthority(const std::string& checker, const std::string& target) {
		int32_t size = sorted_full_nodes_.size();
		if (size < 2) return false;
		int32_t index_range = size < 1000 ? 1 : size / 1000;
		for (int32_t i = 0; i < index_range; ++i) {
			if (sorted_full_nodes_[i] == checker && sorted_full_nodes_[size - 1 - i] == target) {
				return true;
			}
		}
		return false;
	}

	void FullNodeManager::check()
	{
		protocol::LedgerHeader lcl = LedgerManager::Instance().GetLastClosedLedger();

		do 
		{
			if (!sortFullNode(lcl.hash())) {
				LOG_ERROR("Failed to sorted FullNode list by hash value %s", lcl.hash().c_str());
				break;
			}
			last_ledger_seq_ = lcl.seq();

			auto it = full_node_info_.find(local_address_);
			if (it == full_node_info_.end() || !isInspector(local_address_)){
				LOG_INFO("Local address not in full node list or not in the head part");
				break;
			}
			std::string peer;
			if (getPeerAddr(local_address_, peer)) {
				LOG_INFO("Failed to get full node check peer address");
				break;
			}
			// Randomly get the ledger header from the target address
			int64_t random_seq = lcl.seq() - lcl.seq() % full_node_info_.size();
		
			// TODO: send the GetLedger request to peer
			// std::string uri = utils::String::Format("%s://%s", ssl_parameter_.enable_ ? "wss" : "ws", address.ToIpPort().c_str());
			// Connect(uri);

			// TODO: add timer and set impeach func as callback 
			std::string reason = "timeout";
			fullnode_check_timer_ = utils::Timer::Instance().AddTimer(3 * utils::MICRO_UNITS_PER_SEC, 0, [this, peer, reason](int64_t data) {
				impeach(peer, reason);
			});
		} while (false);
		
		return;
	}

	bool FullNodeManager::OnCheck(protocol::WsMessage &msg)
	{
		std::string checker_addr = ""; // TODO: get from msg
		
		protocol::ChainGetLedgerReq req;
		if (!req.ParseFromString(msg.data())) {
			LOG_ERROR("Failed to load ledger header from message, type:(" FMT_I64 "), sequence:(" FMT_I64 ")", msg.type(), msg.sequence());
			return false;
		}

		// Verify authority of checker address  
		if (!verifyCheckAuthority(checker_addr, local_address_)) {
			LOG_ERROR("Failed to verify authority of checker address %s", checker_addr.c_str());
			return false;
		}

		// Get ledger header of seq
		int64_t seq = req.ledger_seq();
		LedgerFrm frm;
		utils::ReadLockGuard guard(Storage::Instance().account_ledger_lock_);
		if (!frm.LoadFromDb(seq)) {
			LOG_ERROR("Failed to load block num:(" FMT_I64 ") from db", seq);
			return false;
		}
		
		// TODO: send response to checker with local ledger header

		return true;
	}

	bool FullNodeManager::checkResponse(protocol::WsMessage &msg)
	{
		// close full node check timer
		utils::Timer::Instance().DelTimer(fullnode_check_timer_);
		
		// decode peer message and compare ledger header
		protocol::LedgerHeader header;
		if (!header.ParseFromString(msg.data())) {
			LOG_ERROR("Failed to load ledger header from message, type:(" FMT_I64 "), sequence:(" FMT_I64 ")", msg.type(), msg.sequence());
			return false;
		}

		LedgerFrm local_frm;
		utils::ReadLockGuard guard(Storage::Instance().account_ledger_lock_);
		if (!local_frm.LoadFromDb(header.seq())) {
			LOG_ERROR("Failed to load block num:(" FMT_I64 ") from db", header.seq());
			return false;
		}
		
		std::string peer; // TODO: get peer from msg
		if (header.hash() != local_frm.GetProtoHeader().hash()) {
			LOG_ERROR("Full node address %s has different block hash in ledger seq(" FMT_I64 ")", peer.c_str(), header.seq());
			std::string reason = "out-sync";
			impeach(peer, reason);
		};
		
		return true;
	}

	bool FullNodeManager::impeach(const std::string& impeach_addr, const std::string& reason)
	{
		auto it = full_node_info_.find(impeach_addr);
		if (it == full_node_info_.end()) {
			LOG_ERROR("Impeach node address %s not exist", impeach_addr.c_str());
			return false;
		}

		protocol::TransactionEnv tran_env;
		Result result;
		result.set_code(protocol::ERRCODE_SUCCESS);
		result.set_desc("");

		protocol::Transaction *tx = tran_env.mutable_transaction();
		tx->set_source_address(local_address_);
		int64_t gas_price = LedgerManager::Instance().GetCurFeeConfig().gas_price();
		tx->set_gas_price(gas_price);
		tx->set_fee_limit(10000);

		protocol::Operation *ope = tx->add_operations();
		ope->set_type(protocol::Operation_Type_PAY_COIN);
		protocol::OperationPayCoin *paycoin = ope->mutable_pay_coin();

		paycoin->set_amount(0);
		paycoin->set_dest_address(General::CONTRACT_FULLNODE_ADDRESS);
		
		Json::Value impeach_json;
		impeach_json["method"] = "impeach";
		impeach_json["params"]["address"] = impeach_addr;
		impeach_json["params"]["ledger_seq"] = last_ledger_seq_;
		impeach_json["params"]["reason"] = reason;
		
		paycoin->set_input(impeach_json.toFastString());
		
		// sign data
		std::string sign_data = priv_key_.Sign(tx->SerializeAsString());
		protocol::Signature *sign = tran_env.add_signatures();
		sign->set_public_key(priv_key_.GetEncPublicKey());
		sign->set_sign_data(sign_data);

		TransactionFrm::pointer ptr = std::make_shared<TransactionFrm>(tran_env);
		if (GlueManager::Instance().OnTransaction(ptr, result)) {
			PeerManager::Instance().Broadcast(protocol::OVERLAY_MSGTYPE_TRANSACTION, tran_env.SerializeAsString());
		}
		if (result.code() != protocol::ERRCODE_SUCCESS) {
			LOG_ERROR("Failed to impeach full node address %s in ledger seq(" FMT_I64 ")", local_address_.c_str(), last_ledger_seq_);
		}
		return true;
	}

	bool FullNodeManager::reward(std::shared_ptr<Environment> env, int64_t fullnode_reward) {
		std::string reward_node = sorted_full_nodes_[0]; // top one of sorted full nodes list
		std::shared_ptr<AccountFrm> account;
		if (!env->GetEntry(reward_node, account)) {
			LOG_ERROR("Failed to get full node account %s", reward_node.c_str());
			return false;
		}
		
		if (!account->AddBalance(fullnode_reward)) {
			LOG_ERROR("Account(%s) allocate reward failed", account->GetAccountAddress().c_str());
			return false;
		}
		return true;
	}
}
