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
	Environment::Environment(mapKV* data, settingKV* settingsData) :
		AtomMap<std::string, AccountFrm>(data), settings_(settingsData)
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
			settings_.Commit();
			return AtomMap<std::string, AccountFrm>::Commit();
		}

		parent_->entries_ = entries_;
		return true;
	}

	void Environment::ClearChangeBuf()
	{
		AtomMap<std::string, AccountFrm>::ClearChangeBuf();
		settings_.ClearChangeBuf();
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

	const std::shared_ptr<Json::Value>& Environment::InitValidators()
	{
		protocol::ValidatorSet currentSets = LedgerManager::Instance().Validators();
		std::shared_ptr<Json::Value> validators;
		for (int i = 0; i < currentSets.validators_size(); i++)
		{
			std::string address = *currentSets.mutable_validators(i);
			(*validators)[i] = address;
		}

		settings_.Set(validatorsKey, validators);
		return validators;
	}

	const std::shared_ptr<Json::Value>& Environment::GetValidators()
	{
		std::shared_ptr<Json::Value> sets;
		settings_.Get(validatorsKey, sets);

		if (!sets)
			sets = InitValidators();

		return sets;
	}
	const std::shared_ptr<Json::Value>& Environment::GetFees()
	{
		std::shared_ptr<Json::Value> fees;
		settings_.Get(feesKey, fees);
		return fees;
	}

	bool Environment::UpdateFeeConfig(const Json::Value &fee_config) {
		std::shared_ptr<Json::Value> fees;
		settings_.Get(feesKey, fees);

		for (Json::Value::iterator it = fees->begin(); it != fees->end(); it++) {
			(*fees)[it.memberName()] = fee_config[it.memberName()];
		}

		return true;
	}

	bool Environment::GetVotedFee(const protocol::FeeConfig &old_fee, protocol::FeeConfig& new_fee) {
		bool change = false;
		new_fee = old_fee;

		std::shared_ptr<Json::Value> fees;
		settings_.Get(feesKey, fees);
		if (!fees) return false;

		for (Json::Value::iterator it = fees->begin(); it != fees->end(); it++) {
			int32_t fee_type = utils::String::Stoi(it.memberName());
			int64_t price = (*fees)[it.memberName()].asInt64();

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

	bool Environment::UpdateNewValidators(const std::shared_ptr<Json::Value>& validators) {
		return settings_.Set(validatorsKey, validators);
	}

	bool Environment::GetVotedValidators(const protocol::ValidatorSet &old_validator, protocol::ValidatorSet& new_validator) {
		std::shared_ptr<Json::Value> sets;
		settings_.Get(validatorsKey, sets);
		if (!sets)
		{
			new_validator = old_validator;
			return false;
		}

		if (sets->size() > 0){
			for (uint32_t i = 0; i < sets->size(); i++) {
				new_validator.add_validators((*sets)[i].asString());
			}

			return true;
		}

		new_validator = old_validator;
		return false;
	}
}