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

#include <sstream>

#include <utils/utils.h>
#include <common/storage.h>
#include <common/pb2json.h>
#include <glue/glue_manager.h>
#include "ledger_manager.h"
#include "ledger_frm.h"
#include "ledgercontext_manager.h"
#include "contract_manager.h"

namespace bumo {
#define COUNT_PER_PARTITION 1000000
	LedgerFrm::LedgerFrm() {
		lpledger_context_ = NULL;
		enabled_ = false;
		apply_time_ = -1;
		total_fee_ = 0;
		total_real_fee_ = 0;
		is_test_mode_ = false;
	}

	
	LedgerFrm::~LedgerFrm() {
	}

	bool LedgerFrm::LoadFromDb(int64_t ledger_seq) {

		bumo::KeyValueDb *db = bumo::Storage::Instance().ledger_db();
		std::string ledger_header;
		int32_t ret = db->Get(ComposePrefix(General::LEDGER_PREFIX, ledger_seq), ledger_header);
		if (ret > 0) {
			ledger_.mutable_header()->ParseFromString(ledger_header);
			return true;
		}
		else if (ret < 0) {
			LOG_ERROR("Get ledger failed, error desc(%s)", db->error_desc().c_str());
			return false;
		}

		return false;
	}


	bool LedgerFrm::AddToDb(WRITE_BATCH &batch) {
		KeyValueDb *db = Storage::Instance().ledger_db();

		batch.Put(bumo::General::KEY_LEDGER_SEQ, utils::String::ToString(ledger_.header().seq()));
		batch.Put(ComposePrefix(General::LEDGER_PREFIX, ledger_.header().seq()), ledger_.header().SerializeAsString());
		
		protocol::EntryList list;
		for (size_t i = 0; i < apply_tx_frms_.size(); i++) {
			const TransactionFrm::pointer ptr = apply_tx_frms_[i];

			protocol::TransactionEnvStore env_store;
			*env_store.mutable_transaction_env() = apply_tx_frms_[i]->GetTransactionEnv();
			env_store.set_ledger_seq(ledger_.header().seq());
			env_store.set_close_time(ledger_.header().close_time());
			env_store.set_error_code(ptr->GetResult().code());
			env_store.set_error_desc(ptr->GetResult().desc());

			batch.Put(ComposePrefix(General::TRANSACTION_PREFIX, ptr->GetContentHash()), env_store.SerializeAsString());
			list.add_entry(ptr->GetContentHash());

			//a transaction success so the transactions trigger by it can store
			if (ptr->GetResult().code() == protocol::ERRCODE_SUCCESS)
				for (size_t j = 0; j < ptr->instructions_.size(); j++){
					protocol::TransactionEnvStore env_sto = ptr->instructions_.at(j);
					env_sto.set_ledger_seq(ledger_.header().seq());
					env_sto.set_close_time(ledger_.header().close_time());
					std::string hash = HashWrapper::Crypto(env_sto.transaction_env().transaction().SerializeAsString());
					batch.Put(ComposePrefix(General::TRANSACTION_PREFIX, hash), env_sto.SerializeAsString());
					list.add_entry(hash);
				}
		}

		batch.Put(ComposePrefix(General::LEDGER_TRANSACTION_PREFIX, ledger_.header().seq()), list.SerializeAsString());

		//save the last tx hash, temporary
		if (list.entry_size() > 0) {
			protocol::EntryList new_last_hashs;
			if (list.entry_size() < General::LAST_TX_HASHS_LIMIT) {
				std::string str_last_hashs;
				int32_t ncount = db->Get(General::LAST_TX_HASHS, str_last_hashs);
				if (ncount < 0) {
					LOG_ERROR("Load last tx hash failed, error desc(%s)", db->error_desc().c_str());
				}

				protocol::EntryList exist_hashs;
				if (ncount > 0 && !exist_hashs.ParseFromString(str_last_hashs)) {
					LOG_ERROR("Parse from string failed");
				}

				for (int32_t i = list.entry_size() - 1; i >= 0; i--) {
					*new_last_hashs.add_entry() = list.entry(i);
				}

				for (int32_t i = 0; 
					i < exist_hashs.entry_size() && new_last_hashs.entry_size() < General::LAST_TX_HASHS_LIMIT;
					i++) { 
					*new_last_hashs.add_entry() = exist_hashs.entry(i);
				}
			} else{
				for (int32_t i = list.entry_size() - 1; i >= list.entry_size() - General::LAST_TX_HASHS_LIMIT; i--) {
					*new_last_hashs.add_entry() = list.entry(i);
				}
			}

			batch.Put(General::LAST_TX_HASHS, new_last_hashs.SerializeAsString());
		}

		if (!db->WriteBatch(batch)){
			PROCESS_EXIT("Write ledger and transaction failed(%s)", db->error_desc().c_str());
		}
		return true;
	}

	bool LedgerFrm::Cancel() {
		enabled_ = false;
		return true;
	}

	bool LedgerFrm::Apply(const protocol::ConsensusValue& request,
		LedgerContext *ledger_context,
		int64_t tx_time_out,
		int32_t &tx_time_out_index) {

		int64_t start_time = utils::Timestamp::HighResolution();
		lpledger_context_ = ledger_context;
		enabled_ = true;
		value_ = std::make_shared<protocol::ConsensusValue>(request);
		uint32_t success_count = 0;
		total_fee_= 0;
		total_real_fee_ = 0;
		environment_ = std::make_shared<Environment>(nullptr);

		for (int i = 0; i < request.txset().txs_size() && enabled_; i++) {
			auto txproto = request.txset().txs(i);
			
			TransactionFrm::pointer tx_frm = std::make_shared<TransactionFrm>(txproto);

			if (!tx_frm->ValidForApply(environment_,!IsTestMode())){
				dropped_tx_frms_.push_back(tx_frm);
				continue;
			}

			//pay fee
			if (!tx_frm->PayFee(environment_, total_fee_)) {
				dropped_tx_frms_.push_back(tx_frm);
				continue;
			}

			ledger_context->transaction_stack_.push_back(tx_frm);
			tx_frm->NonceIncrease(this, environment_);
			if (environment_->useAtomMap_)
				environment_->Commit();

			if (tx_time_out > 0 ) {
				tx_frm->EnableChecked();
				tx_frm->SetMaxEndTime(utils::Timestamp::HighResolution() + tx_time_out);
			} 

			bool ret = tx_frm->Apply(this, environment_);

			//caculate byte fee ,do not store when fee not enough 
			std::string error_info;
			if (tx_frm->IsExpire(error_info)) { //special treatment, return false
				LOG_ERROR("transaction(%s) apply failed. %s, %s",
					utils::String::BinToHexString(tx_frm->GetContentHash()).c_str(), tx_frm->GetResult().desc().c_str(),
					error_info.c_str());
				tx_time_out_index = i;
				return false;
			}else {
				if (!ret ) {
					LOG_ERROR("transaction(%s) apply failed. %s",
						utils::String::BinToHexString(tx_frm->GetContentHash()).c_str(), tx_frm->GetResult().desc().c_str());
					tx_time_out_index = i;
				}
				else {
					tx_frm->environment_->Commit();
				}
			}

			tx_frm->environment_->ClearChangeBuf();
			total_real_fee_ += tx_frm->GetRealFee();
			apply_tx_frms_.push_back(tx_frm);			
			ledger_.add_transaction_envs()->CopyFrom(txproto);
			ledger_context->transaction_stack_.pop_back();
		}
		AllocateFee();
		apply_time_ = utils::Timestamp::HighResolution() - start_time;
		return true;
	}

	bool LedgerFrm::CheckValidation() {
		return true;
	}

	Json::Value LedgerFrm::ToJson() {
		return bumo::Proto2Json(ledger_);
	}

	protocol::Ledger &LedgerFrm::ProtoLedger() {
		return ledger_;
	}

	bool LedgerFrm::Commit(KVTrie* trie, int64_t& new_count, int64_t& change_count) {
		auto batch = trie->batch_;

		if (environment_->useAtomMap_)
		{
			auto entries = environment_->GetData();

			for (auto it = entries.begin(); it != entries.end(); it++){

				if (it->second.type_ == Environment::DEL)
					continue; //there is no delete account function now, not yet

				std::shared_ptr<AccountFrm> account = it->second.value_;
				account->UpdateHash(batch);
				std::string ss = account->Serializer();
				std::string index = DecodeAddress(it->first);
				bool is_new = trie->Set(index, ss);
				if (is_new){
					new_count++;
				}
				else{
					change_count++;
				}
			}
			return true;
		}

		for (auto it = environment_->entries_.begin(); it != environment_->entries_.end(); it++){
			std::shared_ptr<AccountFrm> account = it->second;
			account->UpdateHash(batch);
			std::string ss = account->Serializer();
			std::string index = DecodeAddress(it->first);
			bool is_new = trie->Set(index, ss);
			if (is_new){
				new_count++;
			}
			else{
				change_count++;
			}
		}
		return true;
	}

	bool LedgerFrm::AllocateFee() {
		if (total_fee_ == 0 || IsTestMode()) {
			return true;
		}

		protocol::ValidatorSet set;
		if (!LedgerManager::Instance().GetValidators(ledger_.header().seq() - 1, set)) {
			LOG_ERROR("Get validator failed of ledger seq(" FMT_I64 ")", ledger_.header().seq() - 1);
			return false;
		}
		if (set.validators_size() == 0) {
			LOG_ERROR("Validator should not be empty");
			return false;
		}

		bool average_allocte = false;
		int64_t total_pledge_amount = 0;
		for (int32_t i = 0; i < set.validators_size(); i++) {
			total_pledge_amount += set.validators(i).pledge_coin_amount();
		}
		if (total_pledge_amount == 0) {
			average_allocte = true;
		}

		int64_t tfee = total_fee_;
		std::shared_ptr<AccountFrm> random_account;
		int64_t random_index = ledger_.header().seq() % set.validators_size();
		int64_t average_fee = tfee / set.validators_size();
		for (int32_t i = 0; i < set.validators_size(); i++) {
			std::shared_ptr<AccountFrm> account;
			if (!environment_->GetEntry(set.validators(i).address(), account)) {
				account = AccountFrm::CreatAccountFrm(set.validators(i).address(), 0);
				environment_->AddEntry(account->GetAccountAddress(), account);
			}
			if (random_index == i) {
				random_account = account;
			}

			int64_t fee = 0;
			if (average_allocte) {
				fee = average_fee;
			}
			else {
				fee = tfee*set.validators(i).pledge_coin_amount() / total_pledge_amount;
			}
			LOG_INFO("Account(%s) allocate fee(" FMT_I64 ") left(" FMT_I64 ") in ledger(" FMT_I64 ")", account->GetAccountAddress().c_str(), fee, tfee, ledger_.header().seq());
			tfee -= fee;
			protocol::Account &proto_account = account->GetProtoAccount();
			proto_account.set_balance(proto_account.balance() + fee);
		}
		if (tfee > 0) {
			protocol::Account &proto_account = random_account->GetProtoAccount();
			proto_account.set_balance(proto_account.balance() + tfee);
			LOG_INFO("Account(%s) allocate last fee(" FMT_I64 ") in ledger(" FMT_I64 ")", proto_account.address().c_str(), tfee, ledger_.header().seq());
		}
		if (environment_->useAtomMap_)
			environment_->Commit();
		return true;
	}

	void LedgerFrm::SetTestMode(bool test_mode){
		is_test_mode_ = test_mode;
	}

	bool LedgerFrm::IsTestMode(){
		return is_test_mode_;
	}
}
