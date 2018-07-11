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
#include <common/general.h>
#include "fullnode.h"

namespace bumo {
	FullNode::FullNode() :
		addr_(""),
		addr_hash_(""),
		endpoint_(""),
		apply_time_(0),
		last_check_time_(0) {}

	FullNode::~FullNode() {}

	bool FullNode::loadFromJson(Json::Value& node) {
		addr_ = node["addr"].asString();
		addr_hash_ = HashWrapper::Crypto(addr_);
		endpoint_ = node["endpoint"].asString();
		apply_time_ = node["apply_time"].asInt64();
		last_check_time_ = node["last_check_time"].asInt64();

		Json::Value impeach_list = node["impeach_list"];
		for (unsigned int i = 0; i < impeach_list.size(); ++i)
		{
			Json::Value impeach;
			impeach = impeach_list[i];
			auto keys = impeach.getMemberNames();
			std::string impeach_addr = keys[0];
			Json::Value impeach_info = impeach[impeach_addr];
			ImpeachInfo info;
			info.ledger_seq = impeach_info["ledger_seq"].asInt64();
			info.reason = impeach_info["reason"].asString();
			try {
				impeach_info_.insert(std::make_pair(impeach_addr, info));
			}
			catch (std::exception& e) {
				LOG_ERROR("Insert %s to impeach info map exception, %s", impeach_addr.c_str(), e.what());
				return false;
			}
		}
		return true;
	}

	std::string& FullNode::getAddress() {
		return addr_;
	}
	
	std::string& FullNode::getAddressHash() {
		return addr_hash_;
	}

	std::string& FullNode::getEndPoint() {
		return endpoint_;
	}

	int64_t FullNode::getLastCheckTime() {
		return last_check_time_;
	}

	Json::Value& FullNode::toJson() {
		std::shared_ptr<Json::Value> node;;
		(*node)["addr"] = addr_;
		(*node)["addr_hash"] = addr_hash_;
		(*node)["endpoint"] = endpoint_;
		(*node)["apply_time"] = apply_time_;
		(*node)["last_check_time"] = last_check_time_;

		Json::Value impeach_list;
		Json::Value impeach;
		for (auto it = impeach_info_.begin(); it != impeach_info_.end(); ++it)
		{
			Json::Value value;
			value["ledger_seq"] = it->second.ledger_seq;
			value["reason"] = it->second.reason;
			impeach[it->first] = value;
			impeach_list.append(impeach);
		}
		(*node)["impeach_list"] = impeach_list;
		return *node;
	}

	void FullNode::updateCheckTime() {
		last_check_time_ = utils::Timestamp::Now().timestamp();
	}
	
	bool FullNode::updateImpeach(std::string& report_addr, ImpeachInfo& info) {
		auto range = impeach_info_.equal_range(report_addr);
		for (auto it = range.first; it != range.second; ++it) {
			if (info.ledger_seq <= it->second.ledger_seq) {
				LOG_ERROR("Old impeach info from address(%s) with ledger seq(" FMT_I64 ")", report_addr.c_str(), info.ledger_seq);
				return false;
			}
		}
		impeach_info_.insert(std::make_pair(report_addr, info));
        return true;
	}

	std::string FullNode::getEarliestImpeachAddr() {
		std::string addr;
		int64_t min_seq = utils::MAX_INT64;
		for (auto it = impeach_info_.begin(); it != impeach_info_.end(); ++it) {
			if (it->second.ledger_seq < min_seq) {
				min_seq = it->second.ledger_seq;
				addr = it->first;
			}
		}
		return addr;
	}

	bool FullNode::verifyAddressHash() {
		return true;
	}
}
