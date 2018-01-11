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

#ifndef LEDGER_FRM_H_
#define LEDGER_FRM_H_

#include <utils/utils.h>
#include <proto/cpp/monitor.pb.h>
#include "transaction_frm.h"
#include "glue/transaction_set.h"
#include "account.h"
#include "proto/cpp/consensus.pb.h"

namespace bumo {
	class AccountEntry;
	class LedgerContext;
	class LedgerFrm {
	public:
		typedef std::shared_ptr <LedgerFrm>	pointer;

		LedgerFrm();
		~LedgerFrm();

		protocol::LedgerHeader GetProtoHeader() const {
		  	return ledger_.header();
		}

		protocol::Ledger &ProtoLedger();


		bool Apply(const protocol::ConsensusValue& request, 
			LedgerContext *ledger_context, 
			int64_t tx_time_out, 
			int32_t &tx_time_out_index);
		bool Cancel();

		// void GetSqlTx(std::string &sqltx, std::string &sql_account_tx);

		bool AddToDb(WRITE_BATCH& batch);

		bool LoadFromDb(int64_t seq);

		size_t GetTxCount() {
			return apply_tx_frms_.size();
		}

		size_t GetTxOpeCount() {
			size_t ope_count = 0;
			for (size_t i = 0; i < apply_tx_frms_.size(); i++) {
				const protocol::Transaction &tx = apply_tx_frms_[i]->GetTransactionEnv().transaction();
				ope_count += (tx.operations_size() > 0 ? tx.operations_size() : 1);
			}
			return ope_count;
		}

		bool CheckValidation ();

		Json::Value ToJson();

		bool Commit(KVTrie* trie, int64_t& new_count, int64_t& change_count);

		bool AllocateFee();
		AccountFrm::pointer CreatBookKeeperAccount(const std::string& account_address);
		bool GetVotedFee(protocol::FeeConfig& fee_config);
	private:
		protocol::Ledger ledger_;
	public:
		std::shared_ptr<protocol::ConsensusValue> value_;
		std::vector<TransactionFrm::pointer> apply_tx_frms_;
		std::vector<TransactionFrm::pointer> dropped_tx_frms_;
		std::string sql_;
		std::shared_ptr<Environment> environment_;
		LedgerContext *lpledger_context_;
		int64_t apply_time_;
		bool enabled_;
		int64_t total_fee_;
		int64_t total_real_fee_;
		std::unordered_map<std::string, Json::Value> contracts_output_;
	};
}
#endif //end of ifndef
