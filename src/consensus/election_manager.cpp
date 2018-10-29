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

#include "election_manager.h"

namespace bumo {
	ElectionManager::ElectionManager(): candidate_mpt_(nullptr){

	}

	ElectionManager::~ElectionManager(){
		if (candidate_mpt_){
			delete candidate_mpt_;
			candidate_mpt_ = nullptr;
		}
	}

	bool ElectionManager::Initialize(){
		candidate_mpt_ = new KVTrie();
		auto batch = std::make_shared<WRITE_BATCH>();
		candidate_mpt_->Init(Storage::Instance().account_db(), batch, General::VALIDATOR_CANDIDATE_PREFIX, 1);

		ValidatorCandidatesLoad();

		TimerNotify::RegisterModule(this);
		StatusModule::RegisterModule(this);
		return true;
	}

	bool ElectionManager::Exit() {
		LOG_INFO("Election manager stoping...");

		if (candidate_mpt_) {
			delete candidate_mpt_;
			candidate_mpt_ = nullptr;
		}
		LOG_INFO("election manager stoped. [OK]");
		return true;
	}

	void ElectionManager::OnTimer(int64_t current_time){
	}

	void ElectionManager::OnSlowTimer(int64_t current_time){
	}

	void ElectionManager::GetModuleStatus(Json::Value &data){
	}

	CandidatePtr ElectionManager::GetValidatorCandidate(const std::string& key){
		CandidatePtr candidate = nullptr;

		auto it = validator_candidates_.find(key);
		if (it != validator_candidates_.end()){
			candidate = it->second;
		}

		return candidate;
	}

	bool  ElectionManager::SetValidatorCandidate(const std::string& key, CandidatePtr value){
		if (!value){
			return false;
		}

		try{
			validator_candidates_[key] = value;
		}
		catch (std::exception& e){
			return false;
		}

		return true;
	}

	bool ElectionManager::SetValidatorCandidate(const std::string& key, const protocol::ValidatorCandidate& value){
		try{
			CandidatePtr candidate = std::make_shared<protocol::ValidatorCandidate>(value);
			validator_candidates_[key] = candidate;
		}
		catch (std::exception& e){
			return false;
		}

		return true;
	}

	void ElectionManager::DelValidatorCandidate(const std::string& key){
		validator_candidates_.erase(key);
		to_delete_candidates_.push_back(key);
	}

	bool ElectionManager::ValidatorCandidatesStorage() {
		try{
			for (auto kv : validator_candidates_){
				candidate_mpt_->Set(kv.first, kv.second->SerializeAsString());
			}

			for (auto node : to_delete_candidates_){
				candidate_mpt_->Delete(node);
			}

			to_delete_candidates_.clear();
			candidate_mpt_->UpdateHash();
		}
		catch (std::exception& e){
			return false;
		}

		return true;
	}

	bool ElectionManager::ValidatorCandidatesLoad() {

		try{
			std::vector<std::string> entries;
			candidate_mpt_->GetAll("", entries);

			for (size_t i = 0; i < entries.size(); i++){
				CandidatePtr candidate = std::make_shared<protocol::ValidatorCandidate>();
				candidate->ParseFromString(entries[i]);
				validator_candidates_[candidate->address()] = candidate;
			}
		}
		catch (std::exception& e){
			return false;
		}

		return true;
	}
}
