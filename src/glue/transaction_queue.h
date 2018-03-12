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
        TransactionQueue(uint32_t queue_limit, uint32_t account_txs_limit);
		~TransactionQueue();

		void Import(TransactionFrm::pointer tx,const int64_t& cur_source_nonce);
		protocol::TransactionEnvSet TopTransaction(uint32_t limit);
        uint32_t RemoveTxs(const protocol::TransactionEnvSet& set, bool close_ledger = false);
        void RemoveTxs(std::vector<TransactionFrm::pointer>& txs, bool close_ledger = false);
		void TransactionQueue::CheckTimeout(int64_t current_time, std::vector<TransactionFrm::pointer>& timeout_txs);
        void TransactionQueue::CheckTimeoutAndDel(int64_t current_time);
		bool IsExist(TransactionFrm::pointer tx);


		void PrintAccountQueue(const std::string& account_address);
        void Print();
	private:

		struct PriorityCompare
		{
			TransactionQueue& transaction_queue_;
			/// Compare transaction by nonce height and fee.
			bool operator()(TransactionFrm::pointer const& first, TransactionFrm::pointer const& second) const
			{
                int64_t const& height1 = first->GetNonce() - transaction_queue_.account_nonce_[first->GetSourceAddress()];
                int64_t const& height2 = second->GetNonce() - transaction_queue_.account_nonce_[second->GetSourceAddress()];
				return height1 < height2 || (height1 == height2 && first->GetFee() > second->GetFee());
			}
		};

		

		using PriorityQueue = std::multiset<TransactionFrm::pointer, PriorityCompare>;
		PriorityQueue queue_;
		uint32_t queue_limit_;

		using QueueByNonce = std::map<int64_t, PriorityQueue::iterator>;
		using QueueByAddressAndNonce = std::unordered_map<std::string, QueueByNonce>;
		QueueByAddressAndNonce queue_by_address_and_nonce_;


        //record account system nonce
        std::unordered_map<std::string, int64_t> account_nonce_;

        struct TimePriorityCompare
        {
            /// Compare transaction by incoming time
            bool operator()(TransactionFrm::pointer const& first, TransactionFrm::pointer const& second) const
            {
                return first->GetInComingTime() > second->GetInComingTime();
            }
        };

        //time order
        using TimeQueue = std::multiset<TransactionFrm::pointer, TimePriorityCompare>;
        TimeQueue time_queue_;
        using TimeQueueByNonce = std::map<int64_t, TimeQueue::iterator>;
        using TimeQueueByAddressAndNonce = std::unordered_map<std::string, QueueByNonce>;
        TimeQueueByAddressAndNonce time_queue_by_address_and_nonce_;

		//Maximum number of transaction per account
		uint32_t account_txs_limit_;

        uint32_t queue_cache_limit_;


		std::pair<bool, TransactionFrm::pointer> Remove(const std::string& account_address,const int64_t& nonce);
        std::pair<bool, TransactionFrm::pointer> TimeQueueRemove(const std::string& account_address, const int64_t& nonce);
		std::pair<bool, TransactionFrm::pointer> Remove(QueueByAddressAndNonce::iterator& account_it, QueueByNonce::iterator& tx_it, bool del_empty = true);
        void Insert(QueueByAddressAndNonce::iterator& account_it,TransactionFrm::pointer const& tx);
        void Insert(TransactionFrm::pointer const& tx);
        void TimeQueueInsert(TransactionFrm::pointer const& tx);
		void MoveToQueue(TransactionFrm::pointer const& tx,uint8_t off = 1);
        void RemoveTx(TransactionFrm::pointer tx);

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