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

#ifndef FULLNODE_MANAGER_H_
#define FULLNODE_MANAGER_H_

#include <utils/headers.h>
#include <main/configure.h>
#include "election_manager.h"

namespace bumo {

	bool ElectionManager::Initialize() {
		ElectionConfigure& ecfg = Configure::Instance().election_configure_;

		// Initialize election configuration to ledger db if get configuration failed
		if (!GetConfig(election_config_)) {
			LOG_ERROR("Failed to get election configuration!");
			election_config_.set_max_validators(ecfg.max_validators_);
			election_config_.set_max_candidates(ecfg.max_candidates_);
			election_config_.set_pledge_amount(ecfg.pledge_amount_);
			election_config_.set_validators_refresh_interval(ecfg.validators_refresh_interval_);
			election_config_.set_coin_to_vote_rate(ecfg.coin_to_vote_rate_);
			election_config_.set_fee_to_vote_rate(ecfg.fee_to_vote_rate_);
			election_config_.set_fee_distribution_rate(ecfg.fee_distribution_rate_);
			election_config_.set_penalty_rate(ecfg.penalty_rate_);
		
			// TODO: write configuration to db
			
		}
		LOG_INFO("The election configuration is : %s", election_config_.DebugString().c_str());
		return true;
	}

	bool ElectionManager::Exit() {
		return true;
	}

	void ElectionManager::SetConfig(std::shared_ptr<WRITE_BATCH> batch, const protocol::ElectionConfig &ecfg) {
		batch->Put("election_config", ecfg.SerializeAsString());
	}

	bool ElectionManager::GetConfig(protocol::ElectionConfig &ecfg) {
		std::string key = "election_config";
		auto db = Storage::Instance().account_db();
		std::string str;
		if (!db->Get(key, str)) {
			return false;
		}
		return ecfg.ParseFromString(str);
	}

}

#endif