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

#ifndef FULLNODE_H_
#define FULLNODE_H_

#include <unordered_map>
#include <utils/headers.h>

namespace bumo {
	class FullNode {
	public:
		FullNode();
		~FullNode();

	public:
		struct ImpeachInfo
		{
			int64_t ledger_seq;
			std::string reason;
		};

		bool updateImpeach(std::string& report_addr, ImpeachInfo& info); // update impeach info list
		std::string getEarliestImpeachAddr();
		bool verifyAddressHash(); // validate hash

		std::string& getAddress();
		std::string& getAddressHash();
		std::string& getEndPoint();
		int64_t getLastCheckTime();
		Json::Value& toJson();
		bool loadFromJson(Json::Value& node);

		// ...
	private:
		std::string addr_;//include IP and port
		std::string addr_hash_; // hash of addr_
		std::string endpoint_;
		int64_t apply_time_;
		
		std::unordered_multimap<std::string, ImpeachInfo> impeach_info_; // key: impeach address, value: ImpeachInfo
	};
}
#endif