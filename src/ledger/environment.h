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
#include <utils/atom_map.h>
#include <main/configure.h>
#include "account.h"

namespace bumo {

	class Environment : public AtomMap<std::string, AccountFrm>{
	public:
		std::map<std::string, AccountFrm::pointer> entries_;
		Environment *parent_;
		bool useAtomMap_;

		Environment() = default;
		Environment(Environment const&) = delete;
		Environment& operator=(Environment const&) = delete;

		Environment(mapKV* data, bool dummy);
		Environment(Environment *parent);

		bool GetEntry(const std::string& key, AccountFrm::pointer &frm);
		bool AddEntry(const std::string& key, AccountFrm::pointer frm);
		bool Commit();
		static bool AccountFromDB(const std::string &address, AccountFrm::pointer &account_ptr);
	};
}
#endif