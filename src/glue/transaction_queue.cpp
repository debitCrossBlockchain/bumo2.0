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

#include "transaction_queue.h"
#include <algorithm>

namespace bumo {

	int64_t const QUEUE_TRANSACTION_TIMEOUT = 60 * utils::MICRO_UNITS_PER_SEC;

	TransactionQueue::TransactionQueue(uint32_t queue_limit, uint32_t account_txs_limit)
		: queue_(PriorityCompare{ *this }),
		queue_limit_(queue_limit),
		account_txs_limit_(account_txs_limit)
	{
	}

	TransactionQueue::~TransactionQueue(){}

	
	std::pair<bool, TransactionFrm::pointer> TransactionQueue::Remove(QueueByAddressAndNonce::iterator& account_it, QueueByNonce::iterator& tx_it, bool del_empty){
		TransactionFrm::pointer ptr = nullptr;
		ptr = *tx_it->second;
		queue_.erase(tx_it->second);
		account_it->second.erase(tx_it);

		TimeQueueRemove(ptr->GetSourceAddress(),ptr->GetNonce());

		if (del_empty && account_it->second.empty()){
			account_nonce_.erase(account_it->first);
			queue_by_address_and_nonce_.erase(account_it);            
		}
		return std::move(std::make_pair(true, ptr));
	}
	

	std::pair<bool, TransactionFrm::pointer> TransactionQueue::Remove(const std::string& account_address,const int64_t& nonce){
		TransactionFrm::pointer ptr = nullptr;
		auto account_it =queue_by_address_and_nonce_.find(account_address);
		if (account_it != queue_by_address_and_nonce_.end()){
			auto tx_it = account_it->second.find(nonce);
			if (tx_it != account_it->second.end()){
				ptr = *tx_it->second;
				queue_.erase(tx_it->second);
				account_it->second.erase(tx_it);

				TimeQueueRemove(account_address, nonce);

				if (account_it->second.empty()){
					queue_by_address_and_nonce_.erase(account_it);
					account_nonce_.erase(account_address);
				}
				return std::move(std::make_pair(true, ptr));
			}
		}
		return std::move(std::make_pair(false, ptr));
	}

	std::pair<bool, TransactionFrm::pointer> TransactionQueue::TimeQueueRemove(const std::string& account_address, const int64_t& nonce){
		TransactionFrm::pointer ptr = nullptr;
		auto account_it = time_queue_by_address_and_nonce_.find(account_address);
		if (account_it != time_queue_by_address_and_nonce_.end()){
			auto tx_it = account_it->second.find(nonce);
			if (tx_it != account_it->second.end()){
				ptr = *tx_it->second;
				time_queue_.erase(tx_it->second);
				account_it->second.erase(tx_it);

				if (account_it->second.empty()){
					time_queue_by_address_and_nonce_.erase(account_it);
					account_nonce_.erase(account_address);
				}

				return std::move(std::make_pair(true, ptr));
			}
		}
		return std::move(std::make_pair(false, ptr));
	}


	void TransactionQueue::Insert(TransactionFrm::pointer const& tx){
		// Insert into queue
		auto inserted = queue_by_address_and_nonce_[tx->GetSourceAddress()].insert(std::make_pair(tx->GetNonce(), PriorityQueue::iterator()));
		PriorityQueue::iterator iter = queue_.emplace(tx);
		inserted.first->second = iter;
		TimeQueueInsert(tx);
	}

	void TransactionQueue::Insert(QueueByAddressAndNonce::iterator& account_it, TransactionFrm::pointer const& tx){
		auto inserted = account_it->second.insert(std::make_pair(tx->GetNonce(), PriorityQueue::iterator()));
		PriorityQueue::iterator iter = queue_.emplace(tx);
		inserted.first->second = iter;
		TimeQueueInsert(tx);
	}


	void TransactionQueue::TimeQueueInsert(TransactionFrm::pointer const& tx){
		auto inserted = time_queue_by_address_and_nonce_[tx->GetSourceAddress()].insert(std::make_pair(tx->GetNonce(), TimeQueue::iterator()));
		PriorityQueue::iterator iter = time_queue_.emplace(tx);
		inserted.first->second = iter;
	}



	void TransactionQueue::Import(TransactionFrm::pointer tx, const int64_t& cur_source_nonce){
		utils::WriteLockGuard g(lock_);
		bool replace = false;
		uint32_t account_txs_size = 0;
		bool packed_replace = false;

		account_nonce_[tx->GetSourceAddress()] = cur_source_nonce;

		LOG_TRACE("Import account(%s) transaction(%s) nonce(" FMT_I64 ") fee(" FMT_I64 ")", tx->GetSourceAddress().c_str(), utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetNonce(), tx->GetFee());
		auto account_it = queue_by_address_and_nonce_.find(tx->GetSourceAddress());
		if (account_it != queue_by_address_and_nonce_.end()){

			account_txs_size = account_it->second.size();

			auto tx_it = account_it->second.find(tx->GetNonce());
			if (tx_it != account_it->second.end()){

				if (tx->GetFee() > (*tx_it->second)->GetFee()){
					//remove transaction for replace ,and after insert
					std::string drop_hash = (*tx_it->second)->GetContentHash();
					Remove(account_it, tx_it);
					replace = true;
					if (IsPacked(tx->GetSourceAddress(), tx->GetNonce()))
						packed_replace = true;
					LOG_TRACE("Remove transaction(%s) for replace by transaction(%s) of account(%s) fee(" FMT_I64 ") nonce(" FMT_I64 ") in queue", drop_hash.c_str(), utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetSourceAddress().c_str(), tx->GetFee(), tx->GetNonce());
				}
				else{
					//Discard new transaction
					LOG_TRACE("Discard transaction(%s) of account(%s) fee(" FMT_I64 ") nonce(" FMT_I64 ") because of lower fee  in queue", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetSourceAddress().c_str(), tx->GetFee(), (*tx_it->second)->GetFee(), tx->GetNonce());
					return;
				}
			}
		}

		if (replace || account_txs_size<account_txs_limit_) {
			Insert(tx);
			if (packed_replace)
				ReplacePack(tx->GetSourceAddress(), tx->GetNonce(), tx->GetContentHash());

			//todo...
			while (queue_.size() > queue_limit_){
				TransactionFrm::pointer t = *queue_.rbegin();
				if (!IsPacked(t->GetSourceAddress(), t->GetNonce())){
					Remove(t->GetSourceAddress(), t->GetNonce());
					LOG_TRACE("Remove lowest transaction(%s) of account(%s) fee(" FMT_I64 ") nonce(" FMT_I64 ")  in queue", utils::String::Bin4ToHexString(t->GetContentHash()).c_str(), t->GetSourceAddress().c_str(), t->GetFee(),t->GetNonce());
				}
			}
		}
	}

	protocol::TransactionEnvSet TransactionQueue::TopTransaction(uint32_t limit){
		protocol::TransactionEnvSet set;
		std::unordered_map<std::string, int64_t> topic_seqs;
		utils::WriteLockGuard g(lock_);
		uint64_t i = 0;
		for (auto t = queue_.begin(); set.txs().size() < limit && t != queue_.end(); ++t) {
			const TransactionFrm::pointer& tx = *t;
			if (set.ByteSize() + tx->GetTransactionEnv().ByteSize() >= General::TXSET_LIMIT_SIZE)
				break;
			
			int64_t last_seq = 0;
			do {
				//find this cache
				auto this_iter = topic_seqs.find(tx->GetSourceAddress());
				if (this_iter != topic_seqs.end()) {
					last_seq = this_iter->second;
					break;
				}
				
				last_seq = account_nonce_[tx->GetSourceAddress()];

			} while (false);

			if (tx->GetNonce() > last_seq + 1) {
				LOG_ERROR("The tx seq(" FMT_I64 ") is large than last seq(" FMT_I64 ") + 1", tx->GetNonce(), last_seq);
				break;
			}

			topic_seqs[tx->GetSourceAddress()] = tx->GetNonce();

			*set.add_txs() = tx->GetProtoTxEnv();
			InsertPack(tx->GetSourceAddress(), tx->GetNonce());

			i++;
			LOG_TRACE("top:" FMT_I64 " addr:%s, tx:%s, nonce:" FMT_I64 ", fee:" FMT_I64, i, tx->GetSourceAddress().c_str(), utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetNonce(), tx->GetFee());
		}
		return std::move(set);
	}

	uint32_t TransactionQueue::RemoveTxs(const protocol::TransactionEnvSet& set, bool close_ledger){
		utils::WriteLockGuard g(lock_);
		size_t ret = 0;
		for (int i = 0; i < set.txs_size(); i++) {
			auto txproto = set.txs(i);
			TransactionFrm::pointer tx_frm = std::make_shared<TransactionFrm>(txproto);
			RemoveTx(tx_frm);

			//update system account nonce
			auto it = account_nonce_.find(tx_frm->GetSourceAddress());
			if (close_ledger && it != account_nonce_.end() && it->second < tx_frm->GetNonce())
				it->second = tx_frm->GetNonce();
		}
		return ret;
	}

	void TransactionQueue::RemoveTxs(std::vector<TransactionFrm::pointer>& txs, bool close_ledger){
		utils::WriteLockGuard g(lock_);
		for (auto it = txs.begin(); it != txs.end(); it++){
			RemoveTx(*it);

			//update system account nonce
			auto iter = account_nonce_.find((*it)->GetSourceAddress());
			if (close_ledger && iter != account_nonce_.end() && iter->second < (*it)->GetNonce())
				iter->second = (*it)->GetNonce();
		}
	}

	void TransactionQueue::RemoveTx(TransactionFrm::pointer tx){
		RemovePack(tx->GetSourceAddress(), tx->GetNonce());
		std::pair<bool, TransactionFrm::pointer> result = Remove(tx->GetSourceAddress(), tx->GetNonce());
	}

	std::string TransactionQueue::PackKey(const std::string& account_address, const int64_t& nonce){
		return account_address + std::to_string(nonce);
	}

	void TransactionQueue::InsertPack(const std::string& account_address, const int64_t& nonce){
		packed_txs_[PackKey(account_address,nonce)] = PackReplaceItem();
	}

	void TransactionQueue::ReplacePack(const std::string& account_address, const int64_t& nonce, const std::string& replace_hash){
		auto it = packed_txs_.find(PackKey(account_address, nonce));
		if (it != packed_txs_.end()){
			it->second.Replace(replace_hash);
		}
	}

	bool TransactionQueue::IsPacked(const std::string& account_address, const int64_t& nonce){
		auto it = packed_txs_.find(PackKey(account_address, nonce));
		if (it != packed_txs_.end()){
			return true;
		}
		return false;
	}

	bool TransactionQueue::RemovePack(const std::string& account_address, const int64_t& nonce){
		auto it = packed_txs_.find(PackKey(account_address, nonce));
		if (it != packed_txs_.end()){
			packed_txs_.erase(it);
			return true;
		}
		return false;
	}
	

	void TransactionQueue::CheckTimeout(int64_t current_time, std::vector<TransactionFrm::pointer>& timeout_txs){
		utils::ReadLockGuard g(lock_);
		for (auto it = time_queue_.begin(); it != time_queue_.end();it++){
			if (!(*it)->CheckTimeout(current_time - QUEUE_TRANSACTION_TIMEOUT))
				break;
			timeout_txs.emplace_back(*it);
		}
	}

	void TransactionQueue::CheckTimeoutAndDel(int64_t current_time,std::vector<TransactionFrm::pointer>& timeout_txs){
		utils::WriteLockGuard g(lock_);
		while (true){
			auto it =time_queue_.begin();
			if (!(*it)->CheckTimeout(current_time - QUEUE_TRANSACTION_TIMEOUT))
				break;
			timeout_txs.emplace_back(*it);
			RemoveTx(*it);
		}
	}

	bool TransactionQueue::IsExist(TransactionFrm::pointer tx){
		utils::ReadLockGuard g(lock_);
		auto account_it1 = queue_by_address_and_nonce_.find(tx->GetSourceAddress());
		if (account_it1 != queue_by_address_and_nonce_.end()){
			auto tx_it = account_it1->second.find(tx->GetNonce());
			if (tx_it != account_it1->second.end()){
				TransactionFrm::pointer t = *tx_it->second;
				if (t->GetContentHash() == t->GetContentHash()){
					return true;
				}
			}
		}

		return false;
	}
}

