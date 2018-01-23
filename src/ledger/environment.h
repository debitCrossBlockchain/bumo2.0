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

#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <proto/cpp/chain.pb.h>
#include <proto/cpp/consensus.pb.h>
#include <utils/entry_cache.h>
#include <utils/atom_map.h>
#include <main/configure.h>
#include <json/value.h>
#include "account.h"

namespace bumo {
	class Environment : public AtomMap<std::string, AccountFrm>{
	public:
		const std::string validatorsKey = "validators_key";
		const std::string feesKey		= "fees_key";

		std::map<std::string, AccountFrm::pointer> entries_;
		AtomMap<std::string, Json::Value> settings_;
		typedef AtomMap<std::string, Json::Value>::mapKV  settingKV;

		Environment *parent_;
		bool useAtomMap_;

		Environment() = default;
		Environment(Environment const&) = delete;
		Environment& operator=(Environment const&) = delete;

		Environment(Environment *parent);
		Environment(mapKV* data, settingKV* settingsData);

		bool GetEntry(const std::string& key, AccountFrm::pointer &frm);
		bool AddEntry(const std::string& key, AccountFrm::pointer frm);

		const std::shared_ptr<Json::Value>& InitValidators();
		const std::shared_ptr<Json::Value>& GetValidators();
		const std::shared_ptr<Json::Value>& GetFees();

		bool UpdateFeeConfig(const Json::Value &fee_config);
		bool GetVotedFee(const protocol::FeeConfig &old_fee, protocol::FeeConfig& new_fee);

		bool UpdateNewValidators(const std::shared_ptr<Json::Value>& validators);
		bool GetVotedValidators(const protocol::ValidatorSet &old_validator, protocol::ValidatorSet& new_validator);

		bool Commit();
		void ClearChangeBuf();

		virtual bool GetFromDB(const std::string &address, AccountFrm::pointer &account_ptr);
		static bool AccountFromDB(const std::string &address, AccountFrm::pointer &account_ptr);
	};
}
#endif