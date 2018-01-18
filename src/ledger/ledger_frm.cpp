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

			int64_t time_start = utils::Timestamp::HighResolution();
			tx_frm->SetMaxEndTime(time_start + tx_time_out);

			bool ret = tx_frm->Apply(this, environment_);
			int64_t time_use = utils::Timestamp::HighResolution() - time_start;

			//caculate byte fee ,do not store when fee not enough 
			if (tx_time_out > 0 && time_use > tx_time_out ) { //special treatment, return false
				LOG_ERROR("transaction(%s) apply failed. %s, time out(" FMT_I64 "ms > " FMT_I64 "ms)",
					utils::String::BinToHexString(tx_frm->GetContentHash()).c_str(), tx_frm->GetResult().desc().c_str(),
					time_use / utils::MICRO_UNITS_PER_MILLI, tx_time_out / utils::MICRO_UNITS_PER_MILLI);
				tx_time_out_index = i;
				return false;
			} else {
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
		if (total_fee_ == 0 || IsTestMode()){
			return true;
		}

		protocol::ValidatorSet set;
		if (!LedgerManager::Instance().GetValidators(ledger_.header().seq() - 1, set))	{
			LOG_ERROR("Get validator failed of ledger seq(" FMT_I64 ")", ledger_.header().seq() - 1);
			return false;
		}

		int64_t tfee = total_fee_;
		std::shared_ptr<AccountFrm> random_account;
		int64_t random_index = ledger_.header().seq() % set.validators_size();
		int64_t fee = tfee / set.validators_size();
		for (int32_t i = 0; i < set.validators_size(); i++) {
			std::shared_ptr<AccountFrm> account;
			if (!environment_->GetEntry(set.validators(i), account)) {
				account = CreatBookKeeperAccount(set.validators(i));
			}
			if (random_index == i) {
				random_account = account;
			}
			tfee -= fee;
			protocol::Account &proto_account = account->GetProtoAccount();
			proto_account.set_balance(proto_account.balance() + fee);
		}
		protocol::Account &proto_account = random_account->GetProtoAccount();
		proto_account.set_balance(proto_account.balance() + tfee);
		return true;
	}
	AccountFrm::pointer LedgerFrm::CreatBookKeeperAccount(const std::string& account_address) {
		protocol::Account acc;
		acc.set_address(account_address);
		acc.set_nonce(0);
		acc.set_balance(0);
		AccountFrm::pointer acc_frm = std::make_shared<AccountFrm>(acc);
		acc_frm->SetProtoMasterWeight(1);
		acc_frm->SetProtoTxThreshold(1);
		environment_->AddEntry(acc_frm->GetAccountAddress(), acc_frm);
		LOG_INFO("Add bookeeper account(%s)", account_address.c_str());
		return acc_frm;
	}
	
	bool LedgerFrm::GetVotedFee(const protocol::FeeConfig &old_fee, protocol::FeeConfig& new_fee) {
		
		/*//for test format
		for (auto it = contracts_output_.begin(); it != contracts_output_.end();it++)
		{
			std::string key = it->first;
			Json::Value v= it->second;
			LOG_INFO("victory fee key(%s)--%s",key.c_str(),v.toStyledString().c_str());
		}*/
		/*victory fee format
		[
		null,
		{
			"count" : 3,
		    "enroll_id" : "xx1",
			"fee_type" : 1,
			"price" : 5
		}
		]
		*/

		bool change = false;
		new_fee = old_fee;
		for (Json::Value::iterator iter = fee_config_.begin();
			iter != fee_config_.end();
			iter++) {
			int32_t fee_type = utils::String::Stoi(iter.memberName());
			int64_t price = fee_config_[iter.memberName()].asInt64();
			switch ((protocol::FeeConfig_Type)fee_type) {
			case protocol::FeeConfig_Type_UNKNOWN:
				LOG_ERROR("FeeConfig type error");
				break;
			case protocol::FeeConfig_Type_BYTE_FEE:
				if (new_fee.byte_fee() != price) {
					new_fee.set_byte_fee(price);
					change = true;
				}
				break;
			case protocol::FeeConfig_Type_BASE_RESERVE_FEE:
				if (new_fee.base_reserve() != price) {
					new_fee.set_base_reserve(price);
					change = true;
				}
				break;
			case protocol::FeeConfig_Type_CREATE_ACCOUNT_FEE:
				if (new_fee.create_account_fee() != price) {
					new_fee.set_create_account_fee(price);
					change = true;
				}
				break;
			case protocol::FeeConfig_Type_ISSUE_ASSET_FEE:
				if (new_fee.issue_asset_fee() != price) {
					new_fee.set_issue_asset_fee(price);
					change = true;
				}
				break;
			case protocol::FeeConfig_Type_PAYMENT_FEE:
				if (new_fee.pay_fee() != price) {
					new_fee.set_pay_fee(price);
					change = true;
				}
				break;
			case protocol::FeeConfig_Type_SET_METADATA_FEE:
				if (new_fee.set_metadata_fee() != price) {
					new_fee.set_set_metadata_fee(price);
					change = true;
				}
				break;
			case protocol::FeeConfig_Type_SET_SIGNER_WEIGHT_FEE:
				if (new_fee.set_sigure_weight_fee() != price) {
					new_fee.set_set_sigure_weight_fee(price);
					change = true;
				}
				break;
			case protocol::FeeConfig_Type_SET_THRESHOLD_FEE:
				if (new_fee.set_threshold_fee() != price) {
					new_fee.set_set_threshold_fee(price);
					change = true;
				}
				break;
			case protocol::FeeConfig_Type_PAY_COIN_FEE:
				if (new_fee.pay_coin_fee() != price) {
					new_fee.set_pay_fee(price);
					change = true;
				}
				break;
			default:
				LOG_ERROR("Fee config type(%d) error", fee_type);
				break;
			}

		}

		return change;
	}

	void LedgerFrm::SetTestMode(bool test_mode){
		is_test_mode_ = test_mode;
	}

	bool LedgerFrm::IsTestMode(){
		return is_test_mode_;
	}

	bool LedgerFrm::UpdateFeeConfig(const Json::Value &fee_config) {
		for (Json::Value::iterator iter = fee_config_.begin();
			iter != fee_config_.end();
			iter++) {
			fee_config_[iter.memberName()] = fee_config[iter.memberName()];
		}

		return true;
	}

	bool LedgerFrm::UpdateNewValidators(const Json::Value &validators) {
		new_validators_ = validators;
		return true;
	}

	bool LedgerFrm::GetVotedValidators(const protocol::ValidatorSet &old_validator, protocol::ValidatorSet& new_validator) {
		if (new_validators_.size() > 0){
			for (uint32_t i = 0; i < new_validators_.size(); i++) {
				new_validator.add_validators(new_validators_[i].asString());
			}

			return true;
		} 

		new_validator = old_validator;
		return false;
	}
}
