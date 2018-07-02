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
		bool updateImpeach(std::string& report_addr, int64_t ledger_seq); // update impeach info list
		void updateCheckTime();
		bool verifyAddressHash(); // validate hash
		bool verifySignature();

		std::string& getAddress();
		std::string& getEndPoint();
		int64_t getLastCheckTime();
		// ...
	private:
		std::string addr_;
		std::string endpoint_;
		int64_t applyTime_;
		int64_t lastCheckTime_;
		std::string signature_;
		std::unordered_multimap<std::string, int64_t> impeachInfo_; // key: impeach address, value: ledger_seq
	};
}
#endif