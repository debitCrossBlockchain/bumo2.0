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

    int64_t const CACHE_QUEUE_TRANSACTION_TIMEOUT = 60 * utils::MICRO_UNITS_PER_SEC;

	TransactionQueue::TransactionQueue(uint32_t queue_limit, uint32_t qc_account_slots_limit, uint32_t qc_account_txs_limit)
		: queue_(PriorityCompare{ *this }),
		queue_limit_(queue_limit),
		qc_account_slots_limit_(qc_account_slots_limit),
		qc_account_txs_limit_(qc_account_txs_limit)
	{
	}

	TransactionQueue::~TransactionQueue(){}

	int TransactionQueue::EnqueueCache(TransactionFrm::pointer tx, int64_t next_nonce, bool append_to_queue){
		int ret = 0;
		auto account_it = qc_by_address_and_nonce_.find(tx->GetSourceAddress());

		if (account_it != qc_by_address_and_nonce_.end()){
			auto tx_it = account_it->second.find(tx->GetNonce());
			if (tx_it == account_it->second.end()){
				if (account_it->second.size() < qc_account_txs_limit_){
					InsertCache(tx);
					ret = 2;
					LOG_TRACE("Transaction(%s) is inserted into cache", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str());
				}
				else{
					int64_t max_nonce = account_it->second.rbegin()->first;
					TransactionFrm::pointer max_nonce_tx = *account_it->second.rbegin()->second;
					if (tx->GetNonce() > max_nonce){
						ret = 0;
						LOG_TRACE("Discard insertion of transaction(%s) because of account txs limit in cache", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str());
					}
					else{
						LOG_TRACE("replace transaction(%s) nonce(" FMT_I64 ") by transaction(%s)  nonce(" FMT_I64 ") because of account txs limit in cache",
							utils::String::Bin4ToHexString(max_nonce_tx->GetContentHash()).c_str(), max_nonce,
							utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetNonce());
						//delete
						RemoveCache(account_it, max_nonce,false);
						//insert
						InsertCache(account_it, tx);
						ret = 2;
					}
				}
			}
			else{
				if (tx->GetFee() > (*tx_it->second)->GetFee()){
					//replace
					LOG_TRACE("Transaction(%s) replace transaction(%s) into cache", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), utils::String::Bin4ToHexString((*tx_it->second)->GetContentHash()).c_str());
					RemoveCache(account_it, tx_it,false);
					InsertCache(account_it, tx);
					ret = 1;
					
				}
				else{
					ret = 0;
					LOG_TRACE("Discard replacment of transaction(%s) because of lower fee in cache", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str());
				}
			}

			//append to queue
			int64_t min_nonce = account_it->second.begin()->first;
			if (append_to_queue && queue_.size() < queue_limit_ && min_nonce == next_nonce){
				TransactionFrm::pointer t = *account_it->second.begin()->second;
				MoveToQueue(t,0);
			}
		}
		else{
			if (qc_by_address_and_nonce_.size() < qc_account_slots_limit_){
				InsertCache(tx);
				ret = 2;
				LOG_INFO("Transaction(%s) of new account(%s) is inserted in cache", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetSourceAddress().c_str());
			}
			else{
				ret = 0;
				LOG_WARN("Discard insertion of transaction(%s) of new account(%s) because of accout slots full in cache", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetSourceAddress().c_str());
			}
		}
		return ret;
	}

	int64_t TransactionQueue::GetCahceMinNonce(const std::string& account_address){
		int64_t min_nonce = -1;
		auto account_it = qc_by_address_and_nonce_.find(account_address);
		if (account_it != qc_by_address_and_nonce_.end()){
			min_nonce = (*account_it->second.begin()->second)->GetNonce();
		}
		return min_nonce;
	}

	std::pair<bool, TransactionFrm::pointer> TransactionQueue::Remove(QueueByAddressAndNonce::iterator& account_it, QueueByNonce::iterator& tx_it, bool del_empty){
		TransactionFrm::pointer ptr = nullptr;
		ptr = *tx_it->second;
		queue_.erase(tx_it->second);
		account_it->second.erase(tx_it);

		if (del_empty && account_it->second.empty())
			queue_by_address_and_nonce_.erase(account_it);
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

				if (account_it->second.empty())
					queue_by_address_and_nonce_.erase(account_it);
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

		// Move following transactions from cache to queue
		if (queue_.size() < queue_limit_)
			MoveToQueue(tx);
	}


	std::pair<bool, TransactionFrm::pointer> TransactionQueue::RemoveCache(const std::string& account_address, const int64_t& nonce, bool del_empty){
		TransactionFrm::pointer ptr = nullptr;
		auto account_it = qc_by_address_and_nonce_.find(account_address);
		if (account_it != qc_by_address_and_nonce_.end()){
			auto tx_it = account_it->second.find(nonce);
			if (tx_it != account_it->second.end()){
				ptr = *tx_it->second;
				queue_cache_.erase(tx_it->second);
				account_it->second.erase(tx_it);                

				if (del_empty &&account_it->second.empty())
					qc_by_address_and_nonce_.erase(account_it);
				return std::move(std::make_pair(true, ptr));
			}
		}
		return std::move(std::make_pair(false, ptr));
	}

	std::pair<bool, TransactionFrm::pointer> TransactionQueue::RemoveCache(CacheByAddressAndNonce::iterator& account_it, CacheByNonce::iterator& tx_it, bool del_empty){
		TransactionFrm::pointer ptr = nullptr;
		ptr = *tx_it->second;
		queue_cache_.erase(tx_it->second);
		account_it->second.erase(tx_it);

		if (del_empty && account_it->second.empty())
			qc_by_address_and_nonce_.erase(account_it);
		return std::move(std::make_pair(true, ptr));
	}

	std::pair<bool, TransactionFrm::pointer> TransactionQueue::RemoveCache(CacheByAddressAndNonce::iterator& account_it, const int64_t& nonce, bool del_empty){
		TransactionFrm::pointer ptr = nullptr;
		auto tx_it = account_it->second.find(nonce);
		if (tx_it != account_it->second.end()){
			ptr = *tx_it->second;
			queue_cache_.erase(tx_it->second);
			account_it->second.erase(tx_it);

			if (del_empty && account_it->second.empty())
				qc_by_address_and_nonce_.erase(account_it);
			return std::move(std::make_pair(true, ptr));
		}
		return std::move(std::make_pair(false, ptr));
	}

	void TransactionQueue::InsertCache(TransactionFrm::pointer const& tx){
		auto inserted = qc_by_address_and_nonce_[tx->GetSourceAddress()].insert(std::make_pair(tx->GetNonce(), PriorityQueue::iterator()));
		PriorityQueue::iterator iter = queue_cache_.emplace(tx);
		inserted.first->second = iter;
	}

	void TransactionQueue::InsertCache(CacheByAddressAndNonce::iterator& account_it, TransactionFrm::pointer const& tx){
		auto inserted = account_it->second.insert(std::make_pair(tx->GetNonce(), PriorityQueue::iterator()));
		PriorityQueue::iterator iter = queue_cache_.emplace(tx);
		inserted.first->second = iter;
	}

	void TransactionQueue::MoveToQueue(TransactionFrm::pointer const& tx,uint8_t off ){
		auto account_it = qc_by_address_and_nonce_.find(tx->GetSourceAddress());
		if (account_it != qc_by_address_and_nonce_.end()){
			int64_t nonce = tx->GetNonce() + off;
			auto from_it = account_it->second.find(nonce);
			if (from_it != account_it->second.end()){
				auto tx_it = from_it;
				
				while (tx_it != account_it->second.end() && (*tx_it->second)->GetNonce() == nonce && queue_.size() < queue_limit_){
					TransactionFrm::pointer tran = *tx_it->second;
					//add to queue
					auto inserted = queue_by_address_and_nonce_[tx->GetSourceAddress()].insert(std::make_pair(tran->GetNonce(), PriorityQueue::iterator()));
					PriorityQueue::iterator iter = queue_.emplace(tran);
					inserted.first->second = iter;

					queue_cache_.erase(tx_it->second);
					++tx_it;
					++nonce;
				}
				//delete from cache
				account_it->second.erase(from_it, tx_it);
				if (account_it->second.empty())
					qc_by_address_and_nonce_.erase(tx->GetSourceAddress());
			}
		}

	}
	
	void TransactionQueue::Import(TransactionFrm::pointer tx, const int64_t& cur_source_nonce){
		bool insert_queue = false;
		bool packed_replace = false;
		int64_t next_nonce = -1;

		utils::WriteLockGuard g(lock_);
		LOG_TRACE("Import account(%s) transaction(%s) nonce(" FMT_I64 ") fee(" FMT_I64 ")", tx->GetSourceAddress().c_str(), utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetNonce(), tx->GetFee());
		auto account_it = queue_by_address_and_nonce_.find(tx->GetSourceAddress());
		if (account_it != queue_by_address_and_nonce_.end()){
			auto tx_it = account_it->second.find(tx->GetNonce());
			if (tx_it != account_it->second.end()){

				if (tx->GetFee() > (*tx_it->second)->GetFee()){
					//remove transaction for replace ,and after insert
					std::string drop_hash = (*tx_it->second)->GetContentHash();
					Remove(account_it, tx_it);
					insert_queue = true;
					if (IsPacked(tx->GetSourceAddress(), tx->GetNonce()))
						packed_replace = true;
					LOG_TRACE("Remove transaction(%s) for replace by transaction(%s) in queue", drop_hash.c_str(), utils::String::Bin4ToHexString(tx->GetContentHash()).c_str());
				}
				else{
					//Discard new transaction
					LOG_TRACE("Discard transaction(%s),fee(" FMT_I64 ") is lower than fee(" FMT_I64 ") of transaction(%s) in queue", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetFee(), (*tx_it->second)->GetFee(), utils::String::Bin4ToHexString((*tx_it->second)->GetContentHash()).c_str());
					return;
				}
			}
			else{
				
				int64_t pending_max_nonce = account_it->second.rbegin()->first;
				int64_t min_nonce = GetCahceMinNonce(tx->GetSourceAddress());
				if (tx->GetNonce() == pending_max_nonce + 1) {
					if (min_nonce == tx->GetNonce()){
						insert_queue = false;
						next_nonce = pending_max_nonce + 1;
						LOG_TRACE("Transaction(%s) will be insert into cache", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str());
					}
					else{
						insert_queue = true;
						LOG_TRACE("Transaction(%s) will be insert into queue", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str());
					}
				}
				else if (tx->GetNonce() > pending_max_nonce + 1) {
					insert_queue = false;
					next_nonce = pending_max_nonce + 1;
					LOG_TRACE("Transaction(%s) will be insert into cache", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str());
				}
				else {
					PROCESS_EXIT("Logic error,tx(%s) nonce(" FMT_I64 ") pending_max_nonce(" FMT_I64 ")", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetNonce(), pending_max_nonce);
				}
			}
		}
		else{
			int64_t min_nonce = GetCahceMinNonce(tx->GetSourceAddress());
			if (tx->GetNonce() == cur_source_nonce + 1 ) {
				if (min_nonce == tx->GetNonce()){
					insert_queue = false;
					next_nonce = cur_source_nonce + 1;
					LOG_TRACE("Transaction(%s) will be insert into cache", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str());
				}
				else{
					insert_queue = true;
					LOG_TRACE("Transaction(%s) will be insert into queue", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str());
				}
			}
			else if (tx->GetNonce() > cur_source_nonce + 1){
				insert_queue = false;
				next_nonce = cur_source_nonce + 1;
				LOG_TRACE("Transaction(%s) will be insert into cache", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str());
			}
			else {
				PROCESS_EXIT("Logic error,tx(%s) nonce(" FMT_I64 ") cur_source_nonce(" FMT_I64 ")", utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetNonce(), cur_source_nonce);
			}
		}

		if (insert_queue){
			Insert(tx);
			if (packed_replace){
				ReplacePack(tx->GetSourceAddress(), tx->GetNonce(), tx->GetContentHash());
			}
		}
		else
			EnqueueCache(tx, next_nonce, true);

		//todo...
		while (queue_.size() > queue_limit_){
			TransactionFrm::pointer t = *queue_.rbegin();
			if (!IsPacked(t->GetSourceAddress(),t->GetNonce())){
				Remove(t->GetSourceAddress(), t->GetNonce());
				EnqueueCache(t);
			}
			else{
				break;
			}
		}
	}

	protocol::TransactionEnvSet TransactionQueue::TopTransaction(uint32_t limit){
		protocol::TransactionEnvSet set;
		std::unordered_map<std::string, int64_t> topic_seqs;
		std::vector<TransactionFrm::pointer> invalid_txs;
		utils::WriteLockGuard g(lock_);
		uint64_t i = 0;
		for (auto t = queue_.begin(); set.txs().size() < limit && t != queue_.end(); ++t) {
			const TransactionFrm::pointer& tx = *t;
			if (set.ByteSize() + tx->GetTransactionEnv().ByteSize() >= General::TXSET_LIMIT_SIZE){
				break;
			}
			
			int64_t last_seq = 0;
			do {
				//find this cache
				auto this_iter = topic_seqs.find(tx->GetSourceAddress());
				if (this_iter != topic_seqs.end()) {
					last_seq = this_iter->second;
					break;
				}

				//find global cache
				AccountFrm::pointer account;
				if (Environment::AccountFromDB(tx->GetSourceAddress(), account)) {
					last_seq = account->GetAccountNonce();
				}
			} while (false);

			if (tx->GetNonce() > last_seq + 1) {
				LOG_FATAL("The tx seq(" FMT_I64 ") is large than last seq(" FMT_I64 ") + 1,queue logic error", tx->GetNonce(), last_seq);
				invalid_txs.emplace_back(tx);
				continue;
			}

			if (tx->GetNonce() <= last_seq) {
				LOG_ERROR("The tx seq(" FMT_I64 ") is less or equal of last seq(" FMT_I64 "), remove it", tx->GetNonce(), last_seq);
				invalid_txs.emplace_back(tx);
				continue;
			}
			topic_seqs[tx->GetSourceAddress()] = tx->GetNonce();

			*set.add_txs() = tx->GetProtoTxEnv();
			InsertPack(tx->GetSourceAddress(), tx->GetNonce());

			i++;
			LOG_TRACE("top:" FMT_I64 " addr:%s, tx:%s, nonce:" FMT_I64 ", fee:" FMT_I64, i, tx->GetSourceAddress().c_str(), utils::String::Bin4ToHexString(tx->GetContentHash()).c_str(), tx->GetNonce(), tx->GetFee());
		}
		
		//delete invalid transaction from queue
		for (auto it = invalid_txs.begin(); it != invalid_txs.end();it++){
			Remove((*it)->GetSourceAddress(), (*it)->GetNonce());
		}
		return std::move(set);
	}

	uint32_t TransactionQueue::RemoveTxs(const protocol::TransactionEnvSet& set){
		utils::WriteLockGuard g(lock_);
		size_t ret = 0;
		for (int i = 0; i < set.txs_size(); i++) {
			auto txproto = set.txs(i);
			TransactionFrm::pointer tx_frm = std::make_shared<TransactionFrm>(txproto);
			if (RemovePack(tx_frm->GetSourceAddress(), tx_frm->GetNonce())){
				std::pair<bool, TransactionFrm::pointer> result = Remove(tx_frm->GetSourceAddress(), tx_frm->GetNonce());
				if (!result.first)
					LOG_ERROR("Account(%s) transaction(%s) nonce(" FMT_I64 ") is not in queue to remove",tx_frm->GetSourceAddress(), utils::String::Bin4ToHexString(tx_frm->GetContentHash()),tx_frm->GetNonce());
			}
			else{
				LOG_ERROR("Account(%s) transaction(%s) nonce(" FMT_I64 ") is not in queue to remove", tx_frm->GetSourceAddress(), utils::String::Bin4ToHexString(tx_frm->GetContentHash()), tx_frm->GetNonce());
			}
		}
		return ret;
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
		utils::WriteLockGuard g(lock_);
        while (!queue_cache_.empty()){
			TransactionFrm::pointer tx = *queue_cache_.begin();
            if (!tx->CheckTimeout(current_time - CACHE_QUEUE_TRANSACTION_TIMEOUT))
				break;
			RemoveCache(tx->GetSourceAddress(),tx->GetNonce());
			timeout_txs.emplace_back(tx);
		}
	}

	bool TransactionQueue::IsExist(TransactionFrm::pointer tx){
        utils::ReadLockGuard g(lock_);
		auto account_it = queue_by_address_and_nonce_.find(tx->GetSourceAddress());
		if (account_it != queue_by_address_and_nonce_.end()){
			auto tx_it = account_it->second.find(tx->GetNonce());
			if (tx_it != account_it->second.end()){
				TransactionFrm::pointer t = *tx_it->second;
				if (t->GetContentHash() == t->GetContentHash()){
					return true;
				}
			}
		}

		account_it = qc_by_address_and_nonce_.find(tx->GetSourceAddress());
		if (account_it != qc_by_address_and_nonce_.end()){
			auto tx_it = account_it->second.find(tx->GetNonce());
			if (tx_it != account_it->second.end()){
				TransactionFrm::pointer t = *tx_it->second;
				if (t->GetContentHash() == t->GetContentHash()){
					return true;
				}
			}
		}
		return false;
	}

	/*
	void TransactionQueue::PrintQueue(){
		LOG_INFO("==========================PrintQueue===========================");
		for (auto& e:queue_){
			LOG_INFO("addr:%s, tx:%s, nonce:" FMT_I64 ", fee:" FMT_I64,
			(*e).GetSourceAddress().c_str(),utils::String::Bin4ToHexString((*e).GetContentHash()).c_str(),(*e).GetNonce(),(*e).GetFee());
		}
		LOG_INFO("===============================================================\n");
	}
	void TransactionQueue::PrintQueueByAccount(){
		LOG_INFO("==========================PrintQueueByAccount===========================");
		for (auto& e:queue_by_address_and_nonce_){
			for (auto &i:e.second){
				LOG_INFO("addr:%s, tx:%s, nonce:" FMT_I64 ", fee:" FMT_I64,
					(*i.second)->GetSourceAddress().c_str(), utils::String::Bin4ToHexString((*i.second)->GetContentHash()).c_str(), (*i.second)->GetNonce(), (*i.second)->GetFee() );
			}
		}
		LOG_INFO("=======================================================================\n");
	}
	void TransactionQueue::PrintCache(){
		LOG_INFO("==========================PrintCache===========================");
		for (auto& e : qc_by_address_and_nonce_){
			for (auto &i : e.second){
				LOG_INFO("addr:%s, tx:%s, nonce:" FMT_I64 ", fee:" FMT_I64,
				(*i.second)->GetSourceAddress().c_str() ,utils::String::Bin4ToHexString((*i.second)->GetContentHash()).c_str() ,(*i.second)->GetNonce() ,(*i.second)->GetFee());
			}
		}
		LOG_INFO("=============================================================\n");
	}*/
}

