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
#include <cross/message_channel_manager.h>
#include <cross/base_proposer.h>

#define MAX_CHAIN_ID 1000
#define MAX_REQUEST_BLOCK_NUMS 100
#define MAX_ERROR_TX_COUNT 5
#define MAX_PACK_TX_COUNT 5

namespace bumo {
	class MainProposerManager : public utils::Singleton<MainProposerManager>, public BaseProposer{
		friend class utils::Singleton<bumo::MainProposerManager>;

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
		MainProposerManager();
		~MainProposerManager();
		void UpdateTxResult(const int64_t &error_code, const std::string &error_desc, const std::string& hash);

	private:
		virtual void DoTimerUpdate() override;
		virtual void DoHandleMessageChannel(const protocol::MessageChannel &message_channel) override;
		virtual void DoHandleSenderResult(const TransTask &task_task, const TransTaskResult &task_result) override;


		void UpdateStatus();
		void SendTransaction();
		void UpdateLatestValidates(const int64_t chain_id, utils::StringVector &latest_validates) const;
		void UpdateLatestSeq(const int64_t chain_id, int64_t &seq) const;
		void SortChildSeq(ChildChain &child_chain) const;
		void RequestChainSeq(ChildChain &child_chain) const;

		void OnHandleMainPropser(const protocol::MessageChannel &message_channel);
		void OnHandleChildChallengeSubmitHead(const protocol::MessageChannel &message_channel);
		void OnHandleChildChallengeWithdrawal(const protocol::MessageChannel &message_channel);
		
	private:
		utils::Mutex child_chain_map_lock_;
		ChildChain child_chain_maps_[MAX_CHAIN_ID];
	};

}

#endif
