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

#include <common/storage.h>
#include "ledger_manager.h"
#include "environment.h"

namespace bumo{
	Environment::Environment(mapKV* data, validatorsKV* sets, feesKV* fees) :
		AtomMap<std::string, AccountFrm>(data), validators_(sets), fees_(fees)
	{
		useAtomMap_ = Configure::Instance().ledger_configure_.use_atom_map_;
		parent_ = nullptr;
	}

	Environment::Environment(Environment* parent){

		useAtomMap_ = Configure::Instance().ledger_configure_.use_atom_map_;
		if (useAtomMap_)
		{
			parent_ = nullptr;
			return;
		}

		parent_ = parent;
		if (parent_){
			for (auto it = parent_->entries_.begin(); it != parent_->entries_.end(); it++){
				entries_[it->first] = std::make_shared<AccountFrm>(it->second);
			}
		}
	}

	bool Environment::GetEntry(const std::string &key, AccountFrm::pointer &frm){
		if (useAtomMap_)
			return Get(key, frm);

		if (entries_.find(key) == entries_.end()){
			if (AccountFromDB(key, frm)){
				entries_[key] = frm;
				return true;
			}
			else{
				return false;
			}
		}
		else{
			frm = entries_[key];
			return true;
		}
	}

	bool Environment::Commit(){
		if (useAtomMap_)
		{
			fees_.Commit();
			validators_.Commit();
			return AtomMap<std::string, AccountFrm>::Commit();
		}

		parent_->entries_ = entries_;
		return true;
	}

	void Environment::ClearChangeBuf()
	{
		fees_.ClearChangeBuf();
		validators_.ClearChangeBuf();
		AtomMap<std::string, AccountFrm>::ClearChangeBuf();
	}

	bool Environment::AddEntry(const std::string& key, AccountFrm::pointer frm){
		if (useAtomMap_ == true)
			return Set(key, frm);

		entries_[key] = frm;
		return true;
	}

	bool Environment::GetFromDB(const std::string &address, AccountFrm::pointer &account_ptr)
	{
		return AccountFromDB(address, account_ptr);
	}

	bool Environment::AccountFromDB(const std::string &address, AccountFrm::pointer &account_ptr){

		auto db = Storage::Instance().account_db();
		std::string index = DecodeAddress(address);
		std::string buff;
		if (!LedgerManager::Instance().tree_->Get(index, buff)){
			return false;
		}

		protocol::Account account;
		if (!account.ParseFromString(buff)){
			PROCESS_EXIT("fatal error, account(%s) ParseFromString failed", address.c_str());
		}
		account_ptr = std::make_shared<AccountFrm>(account);
		return true;

	}

	std::shared_ptr<Environment> Environment::NewStackFrameEnv()
	{
		mapKV& data	       = GetActionBuf();
		feesKV& fees       = fees_.GetActionBuf();
		validatorsKV& sets = validators_.GetActionBuf();
		std::shared_ptr<Environment> next = std::make_shared<Environment>(&data, &sets, &fees);

		return next;
	}

	const Environment::validatorsKV& Environment::GetValidators(){
		if (validators_.GetActionBuf().empty()){
			auto sets = LedgerManager::Instance().Validators();
			for (int i = 0; i < sets.validators_size(); i++){
				validators_.SetValue(i, *sets.mutable_validators(i));
			}
		}

		return validators_.GetActionBuf();
	}

	bool Environment::UpdateFeeConfig(const Json::Value &feeConfig) {
		for (auto it = feeConfig.begin(); it != feeConfig.end(); it++) {
			auto type     = (protocol::FeeConfig_Type)utils::String::Stoi(it.memberName());
			auto price = feeConfig[it.memberName()].asInt64();
			fees_.SetValue(type, price);
		}

		return true;
	}

	bool Environment::GetVotedFee(const protocol::FeeConfig &old_fee, protocol::FeeConfig& new_fee) {
		bool change = false;
		new_fee = old_fee;

		auto fees = fees_.GetData();
		if (fees.empty()) return false;

		for (auto kv : fees) {
			int32_t fee_type = kv.first;
			int64_t price = kv.second;

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

	bool Environment::UpdateNewValidators(const Json::Value& validators) {
		for (uint32_t i = 0; i < validators.size(); i++) {
			validators_.SetValue(i, validators[i].asString());
		}
		return true;
	}

	bool Environment::GetVotedValidators(const protocol::ValidatorSet &old_validator, protocol::ValidatorSet& new_validator){
		auto validators = validators_.GetData();

		if (validators.empty())
		{
			new_validator = old_validator;
			return false;
		}

		for (auto kv : validators){
			new_validator.add_validators(kv.second);
		}
		return true;
	}
}