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

#include <utils/headers.h>
#include <common/general.h>

namespace bumo {
	class ElectionManager :
		public utils::Singleton<bumo::ElectionManager>,
		public StatusModule,
		public TimerNotify,
		public utils::Runnable {
	public:
		ElectionManager() {}
		~ElectionManager() {}

		virtual void GetModuleStatus(Json::Value &data);
		virtual void OnTimer(int64_t current_time);
		virtual void OnSlowTimer(int64_t current_time);
		virtual void Run(utils::Thread *this_thread);

		bool Initialize();
		bool Exit();

		struct VoteItem {
			int64_t coin_votes;
			int64_t fee_votes;
		};

		// operation of candidates map
		std::string& GetCandidate(const std::string& addr);
		std::string& GetAllCandidates();
		bool AddCandidate(const std::string& addr);
		bool RemoveCandidate(const std::string& addr);
		bool UpdateCandidate(Json::Value& item);
		bool IsCandidate(const std::string& addr);

		bool AddFeeVotes(const std::string& addr, int64_t votes);
		bool AddCoinVotes(const std::string& addr, int64_t votes);
		bool RemoveCoinVotes(const std::string& addr, int64_t votes);

		std::string& GetProposal(const std::string& proposal_id);
		bool AddProposal(const std::string& proposal);
		bool VoteProposal(const std::string& proposal_id);
		bool DeleteProposal(const std::string& proposal_id);

		bool MarkAbsent(const std::string& addr);
		bool RefreshValidators();

	private:
		std::map<std::string, VoteItem> candidates_map;
		std::multimap<int64_t, std::string> validators_map;
		std::map<std::string, int64_t> absent_map;
		std::map<std::string, std::string> proposals_map;
	};
}


#endif