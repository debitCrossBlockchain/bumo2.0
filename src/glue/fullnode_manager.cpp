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
		last_inspect_seq_(0),
		random_seq_(0),
		last_on_inspect_seq_(0),
		fullnode_check_timer_(0),
		priv_key_(SIGNTYPE_CFCASM2),
		local_address_(""){
		thread_ptr_ = NULL;
	}

	FullNodeManager::~FullNodeManager() {
		if (thread_ptr_){
			delete thread_ptr_;
			thread_ptr_ = NULL;
		}
	}

	bool FullNodeManager::Initialize() {
		if (!priv_key_.From(Configure::Instance().p2p_configure_.node_private_key_)) {
			LOG_ERROR("Initialize node private key failed");
			return false;
		}
		local_address_ = priv_key_.GetEncAddress();

		// Start the thread of FullNodeManager
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("full_node_manager")) {
			return false;
		}

		// Load full nodes from db
		full_node_info_.clear();
		if (!loadAllFullNode()) {
			LOG_ERROR("Initialize full node list failed");
			return false;
		}

		StatusModule::RegisterModule(this);
		TimerNotify::RegisterModule(this);

		LOG_INFO("Full node manager initialized");
		return true;
	}

	void FullNodeManager::GetModuleStatus(Json::Value &data) {
		data["name"] = "fullnode_manager";
		data["local_address"] = local_address_;
		data["last_inspect_seq"] = last_inspect_seq_;
		data["last_on_inspect_seq"] = last_on_inspect_seq_;
		data["random_seq"] = utils::String::ToString(random_seq_);
		Json::Value fullnodes;
		for (auto it = full_node_info_.begin(); it != full_node_info_.end(); it++)
		{
			Json::Value node;
			it->second->toJson(node);
			fullnodes.append(node);
		}
		data["full_nodes"] = fullnodes;
		Json::Value sorted_addr_list;
		for (auto it = sorted_full_nodes_.begin(); it != sorted_full_nodes_.end(); it++)
		{
			sorted_addr_list.append(*it);
		}
		data["sorted_addr_list"] = sorted_addr_list;
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

	bool FullNodeManager::Exit(){
		if (thread_ptr_) {
			thread_ptr_->JoinWithStop();
		}
		return true;
	}

	FullNodePointer FullNodeManager::get(const std::string& key) {
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
			LOG_INFO("Full node address %s already exist", fp->getAddress().c_str());
			return true;
		}
		fp->setAddressHash();
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

	void FullNodeManager::remove(const std::string& key) {
		auto it = full_node_info_.find(key);
		if (it != full_node_info_.end()) {
			full_node_info_.erase(it);
		}
		else {
			LOG_INFO("The full node address(%s) to remove not exist", key.c_str());
		}
		return;
	}

	bool FullNodeManager::update(FullNodePointer fp) {
		std::string addr = fp->getAddress();
		auto it = full_node_info_.find(addr);
		if (it != full_node_info_.end()) {
			it->second = fp;
		}
		else {
			// add full node if not exist
			if (!add(fp)) return false;
		}
		return true;
	}

	void FullNodeManager::getFullNode(const std::string& addr, Json::Value& node) {
		FullNodePointer fp = get(addr);
		if (fp) {
			fp->toJson(node);
		}
	}

	bool FullNodeManager::setFullNode(Json::Value& node, const std::string& operation, std::shared_ptr<WRITE_BATCH> batch) {
		if (operation == "add") {
			FullNodePointer fp = std::make_shared<FullNode>();
			if (!fp->loadFromJson(node)) {
				LOG_ERROR("Failed to load full node info from json, %s", node.toFastString().c_str());
				return false;
			}
			if (!add(fp)) return false;
			batch->Put(utils::String::Format("%s-%s", General::FULLNODE_PREFIX, utils::String::BinToHexString(fp->getAddress()).c_str()), node.toFastString());
		}
		else if (operation == "update") {
			FullNodePointer fp = std::make_shared<FullNode>();
			if (!fp->loadFromJson(node)) {
				LOG_ERROR("Failed to load full node info from json, %s", node.toFastString().c_str());
				return false;
			}
			if (!update(fp)) return false;
			batch->Put(utils::String::Format("%s-%s", General::FULLNODE_PREFIX, utils::String::BinToHexString(fp->getAddress()).c_str()), node.toFastString());
		}
		else if (operation == "remove") {
			std::string addr = node["addr"].asString();
			remove(addr);
			batch->Delete(utils::String::Format("%s-%s", General::FULLNODE_PREFIX, utils::String::BinToHexString(node["addr"].asString()).c_str()));
		}
		else {
			LOG_ERROR("Unknown full node operation, %s", operation.c_str());
			return false;
		}
		LOG_INFO("Set full node done, %s", node.toFastString().c_str());
		return true;
	}

	bool FullNodeManager::updateDb(std::shared_ptr<WRITE_BATCH> batch) {
		if (!Storage::Instance().keyvalue_db()->WriteBatch(*batch)) {
			LOG_ERROR("Write full node batch failed, %s", Storage::Instance().keyvalue_db()->error_desc().c_str());
			return false;
		}
		return true;
	}

	bool FullNodeManager::loadAllFullNode() {
		KVTrie trie;
		auto batch = std::make_shared<WRITE_BATCH>();
		trie.Init(Storage::Instance().keyvalue_db(), batch, General::FULLNODE_PREFIX, 1);
		std::vector<std::string> values;
		trie.GetAll("", values);
		std::map<std::string, FullNodePointer> fullnodes;
		for (size_t i = 0; i < values.size(); i++){
			Json::Value fn_json;
			fn_json.fromString(values[i]);
			FullNodePointer fp = std::make_shared<FullNode>();
			fp->loadFromJson(fn_json);
			if (!update(fp)) return false;
		}
		return true;
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
			std::string key = HashWrapper::Crypto(it->second->getAddressHash() + blockHash);
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

	void FullNodeManager::inspect()
	{
		protocol::LedgerHeader lcl = LedgerManager::Instance().GetLastClosedLedger();

		do 
		{
			// sort by hash of last close ledger
			if (!sortFullNode(lcl.hash())) {
				LOG_ERROR("Failed to sorted FullNode list by hash value %s", lcl.hash().c_str());
				break;
			}
			last_inspect_seq_ = lcl.seq();

			auto it = full_node_info_.find(local_address_);
			if (it == full_node_info_.end() || !isInspector(local_address_)){
				LOG_INFO("Local address %s not in full node list or not in the head part", local_address_.c_str());
				break;
			}
			std::string peer;
			if (!getPeerAddr(local_address_, peer)) {
				LOG_ERROR("Failed to get full node check peer of local address");
				break;
			}

			// check balance of peer
			AccountFrm::pointer peer_account;
			if (!Environment::AccountFromDB(peer, peer_account)) {
				LOG_ERROR("Peer account(%s) not exist", peer.c_str());
				break;
			}
			if (peer_account->GetAccountBalance() < General::FULLNODE_MIN_LOCK_AMOUNT) {
				LOG_ERROR("The peer account balance (" FMT_I64 ") less than (" FMT_I64 ") ", peer_account->GetAccountBalance(), General::FULLNODE_MIN_LOCK_AMOUNT);
				break;
			}

			// Randomly get the ledger header from the target address
			random_seq_ = lcl.seq() - lcl.seq() % full_node_info_.size();
		
			// Send the full node check request to peer
			protocol::FullNodeCheckReq req;
			req.set_ledger_seq(random_seq_);
			req.set_sender(local_address_);
	
			// Impeach when check process timeout
			std::string reason = "timeout";
			fullnode_check_timer_ = utils::Timer::Instance().AddTimer(3 * utils::MICRO_UNITS_PER_SEC, 0, [this, peer, reason](int64_t data) {
				impeach(peer, reason);
			});

			// Connect to peer and send check request, connect or send request failed will cause timeout and trigger impeach
			it = full_node_info_.find(peer);
			if (it == full_node_info_.end()) {
				LOG_ERROR("Internal error, full node check peer not exist");
				break;
			}
			FullNodePointer fp = it->second;
			std::string uri = utils::String::Format("ws://%s:", fp->getEndPoint().c_str());
			PeerManager::Instance().ConsensusNetwork().Connect(uri); 
			PeerManager::Instance().SendRequest(uri, protocol::OVERLAY_MSGTYPE_FULLNODE_CHECK, req.SerializeAsString());
		} while (false);
		
		return;
	}

	bool FullNodeManager::OnInspect(protocol::WsMessage &msg, int64_t conn_id)
	{
		protocol::FullNodeCheckReq req;
		if (!req.ParseFromString(msg.data())){
			LOG_ERROR("Parse full node check request failed");
			return false;
		}
		
		std::string checker_addr = req.sender();
		protocol::FullNodeCheckResp resp;
		resp.set_error_code(protocol::ERRCODE_SUCCESS);
		resp.set_error_desc("");

		last_on_inspect_seq_ = req.ledger_seq();

		do 
		{
			// Verify authority of checker address  
			if (!verifyCheckAuthority(checker_addr, local_address_)) {
				resp.set_error_code(protocol::ERRCODE_INVALID_PARAMETER);
				resp.set_error_desc(utils::String::Format("Verify check authority failed, peer address %s has no permission", checker_addr.c_str()));
				LOG_ERROR("%s", resp.error_desc().c_str());
				break;
			}

			// Get ledger header of seq
			LedgerFrm frm;
			utils::ReadLockGuard guard(Storage::Instance().account_ledger_lock_);
			if (!frm.LoadFromDb(req.ledger_seq())) {
				resp.set_error_code(protocol::ERRCODE_NOT_EXIST);
				resp.set_error_desc(utils::String::Format("Failed to get ledger (" FMT_I64 ") from db", req.ledger_seq()));
				LOG_ERROR("%s", resp.error_desc().c_str());
				break;
			}

			protocol::LedgerHeader* header = resp.mutable_header();
			header->CopyFrom(frm.GetProtoHeader());
		} while (false);
		
		PeerManager::Instance().SendRequest(conn_id, protocol::OVERLAY_MSGTYPE_FULLNODE_CHECK, resp.SerializeAsString());

		return true;
	}

	bool FullNodeManager::OnInspectResponse(protocol::WsMessage &msg, int64_t conn_id)
	{
		protocol::FullNodeCheckResp resp;
		if (!resp.ParseFromString(msg.data())) {
			LOG_ERROR("Parse full node check response failed");
			return false;
		}

		// close full node check timer
		utils::Timer::Instance().DelTimer(fullnode_check_timer_);

		LedgerFrm local_frm;
		utils::ReadLockGuard guard(Storage::Instance().account_ledger_lock_);
		if (!local_frm.LoadFromDb(random_seq_)) {
			LOG_ERROR("Failed to load block num:(" FMT_I64 ") from db", random_seq_);
			return false;
		}
		
		std::string peer;
		if (!getPeerAddr(local_address_, peer)) {
			LOG_ERROR("Failed to get full node check peer address");
			return false;
		}

		if (local_frm.GetProtoHeader().SerializeAsString() != resp.header().SerializeAsString()) {
			LOG_ERROR("Full node address %s has different block hash in ledger seq(" FMT_I64 ")", peer.c_str(), random_seq_);
			std::string reason = "out-sync";
			return impeach(peer, reason);
		}

		// remove the earliest one from impeach list
		auto it = full_node_info_.find(peer);
		if (it == full_node_info_.end()) {
			LOG_ERROR("Internal error, full node check peer %s not exist", peer.c_str());
			return false;
		}

		std::string addr = it->second->getEarliestImpeachAddr();
		if (!addr.empty()) {
			return unimpeach(peer, addr);
		}
		else {
			// disconnect
			PeerManager::Instance().ConsensusNetwork().Disconnect(conn_id);
		}
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
		Json::Value info;
		info["ledger_seq"] = last_inspect_seq_;
		info["reason"] = reason;
		impeach_json["impeach"] = info;
		
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
		else {
			LOG_ERROR("Failed to impeach full node address %s in ledger seq(" FMT_I64 ")", local_address_.c_str(), last_inspect_seq_);
			return false;
		}
		return true;
	}

	bool FullNodeManager::unimpeach(const std::string& address, const std::string& unimpeach_addr)
	{
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
		impeach_json["method"] = "unimpeach";
		impeach_json["params"]["address"] = address;
		impeach_json["params"]["unimpeach_addr"] = unimpeach_addr;

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
		else {
			LOG_ERROR("Failed to unimpeach full node address %s from %s", unimpeach_addr.c_str(), address.c_str());
			return false;
		}
		return true;
	}

	bool FullNodeManager::reward(std::shared_ptr<Environment> env, int64_t fullnode_reward) {
		if (sorted_full_nodes_.empty()) return true;
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
