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

#ifndef TRANSACTION_QUEUE_
#define TRANSACTION_QUEUE_

#include <proto/cpp/overlay.pb.h>
#include <proto/cpp/chain.pb.h>
#include <ledger/transaction_frm.h>
#include "utils/thread.h"
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>

namespace bumo {

	class TransactionQueue{
	public:
		TransactionQueue(uint32_t queue_limit, uint32_t qc_account_slots_limit, uint32_t qc_account_txs_limit);
		~TransactionQueue();

		void Import(TransactionFrm::pointer tx,const int64_t& cur_source_nonce);
		protocol::TransactionEnvSet TopTransaction(uint32_t limit);
		uint32_t RemoveTxs(const protocol::TransactionEnvSet& set);
		void TransactionQueue::CheckTimeout(int64_t current_time, std::vector<TransactionFrm::pointer>& timeout_txs);
        bool IsExist(TransactionFrm::pointer tx);
		//void PrintQueue();
		//void PrintQueueByAccount();
		//void PrintCache();
	private:

		struct PriorityCompare
		{
			TransactionQueue& transaction_queue_;
			/// Compare transaction by nonce height and fee.
			bool operator()(TransactionFrm::pointer const& first, TransactionFrm::pointer const& second) const
			{
				uint64_t const& height1 = first->GetNonce() - transaction_queue_.queue_by_address_and_nonce_[first->GetSourceAddress()].begin()->first;
				uint64_t const& height2 = second->GetNonce() - transaction_queue_.queue_by_address_and_nonce_[second->GetSourceAddress()].begin()->first;
				return height1 < height2 || (height1 == height2 && first->GetFee() > second->GetFee());
			}
		};

		struct CachePriorityCompare
		{
			/// Compare transaction by incoming time
			bool operator()(TransactionFrm::pointer const& first, TransactionFrm::pointer const& second) const
			{
				return (first->GetInComingTime() > second->GetInComingTime());
			}
		};        
	   

		using PriorityQueue = std::multiset<TransactionFrm::pointer, PriorityCompare>;
		PriorityQueue queue_;
		uint32_t queue_limit_;

		using QueueByNonce = std::map<int64_t, PriorityQueue::iterator>;
		using QueueByAddressAndNonce = std::unordered_map<std::string, QueueByNonce>;
		QueueByAddressAndNonce queue_by_address_and_nonce_;

		using CachePriorityQueue = std::multiset<TransactionFrm::pointer, CachePriorityCompare>;
		CachePriorityQueue queue_cache_;

		using CacheByNonce = std::map<int64_t, CachePriorityQueue::iterator>;
		using CacheByAddressAndNonce = std::unordered_map<std::string, CacheByNonce>;
		CacheByAddressAndNonce qc_by_address_and_nonce_;
		//Maximum number of all account
		uint32_t qc_account_slots_limit_;
		//Maximum number of transaction per account
		uint32_t qc_account_txs_limit_;      
		// life time in queue cache
		uint64_t life_time_;

		/*
		return: 0 discard ; 1 replace ; 2 insert 
		*/
		int EnqueueCache(TransactionFrm::pointer tx, int64_t next_nonce = -1, bool append_to_queue = false);
		

		std::pair<bool, TransactionFrm::pointer> Remove(const std::string& account_address,const int64_t& nonce);
		std::pair<bool, TransactionFrm::pointer> Remove(QueueByAddressAndNonce::iterator& account_it, QueueByNonce::iterator& tx_it, bool del_empty = true);
		void Insert(TransactionFrm::pointer const& tx);
		void MoveToQueue(TransactionFrm::pointer const& tx,uint8_t off = 1);

		int64_t GetCahceMinNonce(const std::string& account_address);
		void InsertCache(TransactionFrm::pointer const& tx);
		void InsertCache(CacheByAddressAndNonce::iterator& account_it, TransactionFrm::pointer const& tx);
		std::pair<bool, TransactionFrm::pointer> RemoveCache(const std::string& account_address, const int64_t& nonce, bool del_empty = true);
		std::pair<bool, TransactionFrm::pointer> RemoveCache(CacheByAddressAndNonce::iterator& account_it, const int64_t& nonce, bool del_empty = true);
		std::pair<bool, TransactionFrm::pointer> RemoveCache(CacheByAddressAndNonce::iterator& account_it, CacheByNonce::iterator& tx_it, bool del_empty = true);

		struct PackReplaceItem
		{
			PackReplaceItem(){}
			PackReplaceItem(const std::string& replaced_hash) :replaced_hash_(replaced_hash_){}
			PackReplaceItem(const PackReplaceItem& item){
				replaced_hash_ = item.replaced_hash_;
			}
			std::string replaced_hash_;
			bool IsReplace(){ return !replaced_hash_.empty(); }
			void Replace(const std::string& hash){ replaced_hash_ = hash; }
		};
		//key:account_address+nonce ,value
		std::unordered_map<std::string, PackReplaceItem> packed_txs_;
		std::string PackKey(const std::string& account_address, const int64_t& nonce);
		void InsertPack(const std::string& account_address, const int64_t& nonce);
		void ReplacePack(const std::string& account_address, const int64_t& nonce, const std::string& replace_hash);
		bool IsPacked(const std::string& account_address, const int64_t& nonce);
		bool RemovePack(const std::string& account_address, const int64_t& nonce);
		utils::ReadWriteLock lock_;
	};
}

#endif