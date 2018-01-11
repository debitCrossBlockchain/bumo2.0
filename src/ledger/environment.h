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
#include <utils/entry_cache.h>
#include "account.h"

namespace bumo {

	//class Environment :public utils::EntryCache<std::string, AccountFrm, StringSort> {

	//	virtual bool LoadValue(const std::string &address, AccountFrm::pointer &frm) {
	//		AccountFrm::pointer account_pt;
	//		if (!Environment::AccountFromDB(address, account_pt)) {
	//			return false;
	//		}
	//		else
	//			frm = account_pt;
	//		return true;
	//	}

	//public:

	//	Environment() {
	//		parent_ = nullptr;
	//	}

	//	Environment(std::shared_ptr<Environment> p) {
	//		parent_ = p;
	//	}

	//	~Environment(){
	//	}

	//	void Commit(){
	//		if (parent_ == nullptr){
	//			return;
	//		}
	//		for (auto it = entries_.begin(); it != entries_.end(); it++) {
	//			std::shared_ptr<AccountFrm> account = it->second.value_;
	//			parent_->entries_[it->first] = it->second;
	//		}
	//	}

	//	static bool AccountFromDB(const std::string &address, AccountFrm::pointer &account_ptr);
	//	static int64_t time_;
	//};

	class Environment{
	public:
		std::map<std::string, AccountFrm::pointer> entries_;
		Environment *parent_;
		//
		Environment() = delete;
		Environment(Environment const&) = delete;
		Environment& operator=(Environment const&) = delete;

		Environment(Environment *parent);
		bool GetEntry(const std::string& key, AccountFrm::pointer &frm);
		bool AddEntry(const std::string& key, AccountFrm::pointer frm);
		void Commit();
		static bool AccountFromDB(const std::string &address, AccountFrm::pointer &account_ptr);
	};
}
#endif