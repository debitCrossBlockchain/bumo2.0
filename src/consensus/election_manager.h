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

#ifndef ELECTION_MANAGER_H_
#define ELECTION_MANAGER_H_

#include <vector>
#include <string>
#include <unordered_map>
#include <proto/cpp/chain.pb.h>
#include <utils/headers.h>
#include <common/general.h>
#include <common/storage.h>
#include "ledger/kv_trie.h"

namespace bumo {
	typedef std::shared_ptr<protocol::ValidatorCandidate> CandidatePtr;

	class ElectionManager : 
		public utils::Singleton<bumo::ElectionManager>,
		public bumo::TimerNotify,
		public bumo::StatusModule  {
		friend class utils::Singleton<bumo::ElectionManager>;

	private:
		ElectionManager();
		~ElectionManager();

	private:
		utils::ReadWriteLock candidates_mutex_;
		std::vector<std::string> to_delete_candidates_;
		std::unordered_map<std::string, CandidatePtr> validator_candidates_;
		KVTrie* candidate_mpt_;

	public:
		bool Initialize();
		bool Exit();

		bool SetValidatorCandidate(const std::string& key, CandidatePtr value);
		bool SetValidatorCandidate(const std::string& key, const protocol::ValidatorCandidate& value);
		CandidatePtr GetValidatorCandidate(const std::string& key);
		void DelValidatorCandidate(const std::string& key);

		bool ValidatorCandidatesStorage();
		bool ValidatorCandidatesLoad();

		virtual void OnTimer(int64_t current_time);
		virtual void OnSlowTimer(int64_t current_time);
		virtual void GetModuleStatus(Json::Value &data);

	};

}

#endif
