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

#include "fullnode.h"

namespace bumo {
	FullNode::FullNode() {
		applyTime_ = 0;
		lastCheckTime_ = 0;
	}
	FullNode::~FullNode() {}

	std::string& FullNode::getAddress() {
		return addr_;
	}
	
	std::string& FullNode::getEndPoint() {
		return endpoint_;
	}

	int64_t FullNode::getLastCheckTime() {
		return lastCheckTime_;
	}

	void FullNode::updateCheckTime() {
		lastCheckTime_ = utils::Timestamp::Now().timestamp();
	}
	
	bool FullNode::updateImpeach(std::string& report_addr, int64_t ledger_seq) {
		int64_t now = utils::Timestamp::Now().timestamp();
		auto range = impeachInfo_.equal_range(report_addr);
		for (auto it = range.first; it != range.second; ++it) {
			if (ledger_seq <= it->second) {
				LOG_ERROR("Old impeach info from address(%s) with ledger seq(" FMT_I64 ")", report_addr, ledger_seq);
				return false;
			}
		}
		impeachInfo_.insert(std::make_pair(report_addr, ledger_seq));
        return true;
	}

	bool FullNode::verifyAddressHash() {
		return true;
	}

	bool FullNode::verifySignature() {
		return true;
	}
}
