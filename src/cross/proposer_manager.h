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

#ifndef PROPOSER_MANAGER_H_
#define PROPOSER_MANAGER_H_

#include <utils/singleton.h>
#include <utils/thread.h>
#include <cross/cross_utils.h>
#include<cross/message_channel_manager.h>

#define MAX_CHAIN_ID 1000
#define MAX_REQUEST_BLOCK_NUMS 100
#define MAX_ERROR_TX_COUNT 5
#define MAX_PACK_TX_COUNT 5
#define PROPOSER_PERIOD 15

namespace bumo {

	class ProposerManager :public utils::Singleton<ProposerManager>,
		public bumo::IMessageChannelConsumer, public ITransactionSenderNotify,
		public utils::Runnable{
		friend class utils::Singleton<bumo::ProposerManager>;

		typedef std::map<int64_t, protocol::LedgerHeader> LedgerMap;
		typedef struct tagChildChain
		{
			int64_t chain_id;
			LedgerMap ledger_map;
			int64_t recv_max_seq;
			int64_t cmc_latest_seq;
			int64_t error_tx_times = 0;
			utils::StringList tx_history;
			utils::StringVector cmc_latest_validates;
		public:
			void Reset() {
				chain_id = -1;
				ledger_map.clear();
				recv_max_seq = -1;
				cmc_latest_seq = -1;
				cmc_latest_validates.clear();
			}
		}ChildChain;

	public:
		ProposerManager();
		~ProposerManager();

		bool Initialize();
		bool Exit();
		void UpdateTransactionResult(const int64_t &error_code,const std::string &error_desc,const std::string& hash);

	private:
		virtual void Run(utils::Thread *thread) override;
		virtual void HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel) override;
		virtual void HandleTransactionSenderResult(const TransTask &task_task, const TransTaskResult &task_result) override;

		void UpdateLatestStatus();
		void UpdateLatestValidates(const int64_t chain_id, utils::StringVector &latest_validates);
		void UpdateLatestSeq(const int64_t chain_id, int64_t &seq);
		void SortChildSeq(ChildChain &child_chain);
		void RequestChainSeq(int64_t chain_id, int64_t seq);
		void ProposeBlocks();
		void BreakProposer(const std::string &error_des);

	private:
		bool enabled_;
		utils::Thread *thread_ptr_;

		utils::Mutex child_chain_map_lock_;
		ChildChain child_chain_maps_[MAX_CHAIN_ID];

		int64_t last_propose_time_;
		bool main_chain_;
		std::string source_address_;
	};

}

#endif
