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
#include <proto/cpp/chain.pb.h>
#include <ledger/ledger_manager.h>
#include <common/network.h>

#include "fullnode.h"
#include "fullnode_manager.h"
#include "glue_manager.h"


namespace bumo {
	FullNodeManager::FullNodeManager() :
		last_ledger_seq_(0),
		fullnode_check_timer_(0),
		priv_key_(SIGNTYPE_CFCASM2),
		local_address_("") {}

	FullNodeManager::~FullNodeManager() {}

	bool FullNodeManager::Initialize() {
		if (!priv_key_.From(Configure::Instance().p2p_configure_.node_private_key_)) {
			LOG_ERROR("Initialize node private key failed");
			return false;
		}
		local_address_ = priv_key_.GetEncAddress();

		return true;
	}

	FullNodePointer FullNodeManager::get(std::string& key) {
		auto it = fullNodeInfo_.find(key);
		if (it != fullNodeInfo_.end()) {
			return it->second;
		}
		else {
			return nullptr;
		}
	}

	bool FullNodeManager::add(FullNode& fn) {
		if (fullNodeInfo_.find(fn.getAddress()) != fullNodeInfo_.end()) {
			FullNodePointer fp(&fn);
			fullNodeInfo_.insert(std::make_pair(fn.getAddress(), fp));
		}
		else {
			LOG_ERROR("Node address(%s) already exist", fn.getAddress().c_str());
			return false;
		}
		return true;
	}

	bool FullNodeManager::remove(std::string& key) {
		auto it = fullNodeInfo_.find(key);
		if (it != fullNodeInfo_.end()) {
			fullNodeInfo_.erase(it);
		}
		else {
			LOG_ERROR("FullNode address(%s) not exist", key.c_str());
			return false;
		}
		return true;
	}

	bool FullNodeManager::isHead1In1000(const std::string& addr, std::string& peer) {
		int32_t size = sortedFullNodes_.size();
		if (size < 2) return false;
		int32_t index_range = size < 1000 ? 1 : size / 1000;
		for (int32_t i = 0; i < index_range; ++i) {
			if (sortedFullNodes_[i] == addr) {
				peer = sortedFullNodes_[size - 1 - i];
				return true;
			}
		}
		return false;
	}

	bool FullNodeManager::isTail1In1000(const std::string& addr, std::string& peer) {
		int32_t size = sortedFullNodes_.size();
		if (size < 2) return false;
		int32_t index_range = size < 1000 ? 1 : size / 1000;
		for (int32_t i = 0; i < index_range; ++i) {
			if (sortedFullNodes_[size - 1 - i] == addr) {
				peer = sortedFullNodes_[i];
				return true;
			}
		}
		return false;
	}

	bool FullNodeManager::sortFullNode(const std::string& blockHash){
		sortedFullNodes_.clear();
		std::map<std::string, std::string> sorted_map;
		for (auto it = fullNodeInfo_.begin(); it != fullNodeInfo_.end(); ++it) {
			std::string key = HashWrapper::Crypto(HashWrapper::Crypto(it->first) + blockHash);
			sorted_map.insert(std::make_pair(key, it->first));
		}
		for (auto it = sorted_map.begin(); it != sorted_map.end(); ++it) {
			sortedFullNodes_.push_back(it->second);
		}
		return true;
	}

	bool FullNodeManager::verifyCheckAuthority(const std::string& checker, const std::string& target) {
		int32_t size = sortedFullNodes_.size();
		if (size < 2) return false;
		int32_t index_range = size < 1000 ? 1 : size / 1000;
		for (int32_t i = 0; i < index_range; ++i) {
			if (sortedFullNodes_[i] == checker && sortedFullNodes_[size - 1 - i] == target) {
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

			auto it = fullNodeInfo_.find(local_address_);
			std::string peer;
			if (it == fullNodeInfo_.end() || !isHead1In1000(local_address_, peer)){
				LOG_INFO("Local address not in full node list or not in the head part");
				break;
			}
		
			// get ledger header randomly from target address
			int64_t random_seq = lcl.seq() - lcl.seq() % fullNodeInfo_.size();
		
			// TODO: send GetLedger request to peer

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
		protocol::TransactionEnv tran_env;
		Result result;
		result.set_code(protocol::ERRCODE_SUCCESS);
		result.set_desc("");

		
		protocol::Transaction *tx = tran_env.mutable_transaction();
		tx->set_source_address(local_address_);
		tx->set_fee_limit(100000);
		tx->set_gas_price(1000);

		protocol::Operation *ope = tx->add_operations();
		ope->set_type(protocol::Operation_Type_PAY_COIN);
		protocol::OperationPayCoin *paycoin = ope->mutable_pay_coin();

		paycoin->set_amount(0);
		paycoin->set_dest_address(General::CONTRACT_FULLNODE_ADDRESS);
		
		Json::Value impeach_json;
		impeach_json["method"] = "impeach";
		impeach_json["params"]["address"] = impeach_addr;
		auto it = fullNodeInfo_.find(impeach_addr);
		if (it == fullNodeInfo_.end()) {
			LOG_ERROR("Impeach node address %s not exist", impeach_addr.c_str());
			return false;
		}

		impeach_json["params"]["ledger_seq"] = last_ledger_seq_;
		impeach_json["params"]["reason"] = reason;
		
		paycoin->set_input(impeach_json.toFastString());
		
		// sign data
		std::string sign_data = priv_key_.Sign(tx->SerializeAsString());
		protocol::Signature *sign = tran_env.add_signatures();
		sign->set_public_key(priv_key_.GetEncPublicKey());
		sign->set_sign_data(sign_data);

		TransactionFrm::pointer ptr = std::make_shared<TransactionFrm>(tran_env);
		GlueManager::Instance().OnTransaction(ptr, result);
		if (result.code() != protocol::ERRCODE_SUCCESS) {
			LOG_ERROR("Failed to impeach full node address %s in ledger seq(" FMT_I64 ")", local_address_.c_str(), last_ledger_seq_);
		}
		return true;
	}

	bool FullNodeManager::reward(std::shared_ptr<Environment> env, int64_t fullnode_reward) {
		std::string reward_node = sortedFullNodes_[0]; // top one of sorted full nodes list
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
