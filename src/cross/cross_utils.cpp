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


#include<cross/cross_utils.h>
namespace bumo {

	CrossUtilsManager::CrossUtilsManager(){}
	CrossUtilsManager::~CrossUtilsManager(){}

	void  CrossUtilsManager::CallContract(ContractTestParameter &test_parameter, std::string &reply){

		int32_t error_code = protocol::ERRCODE_SUCCESS;
		std::string error_desc;
		AccountFrm::pointer acc = NULL;

		Json::Value reply_json = Json::Value(Json::objectValue);
		Json::Value &result = reply_json["result"];

		do {
			if (!test_parameter.contract_address_.empty()) {
				if (!Environment::AccountFromDB(test_parameter.contract_address_, acc)) {
					error_code = protocol::ERRCODE_NOT_EXIST;
					error_desc = utils::String::Format("Account(%s) is not existed", test_parameter.contract_address_.c_str());
					LOG_ERROR("Failed to load the account from the database. %s", error_desc.c_str());
					break;
				}

				test_parameter.code_ = acc->GetProtoAccount().contract().payload();
			}

			if (test_parameter.code_.empty()) {
				error_code = protocol::ERRCODE_NOT_EXIST;
				error_desc = utils::String::Format("Account(%s) has no contract code", test_parameter.contract_address_.c_str());
				LOG_ERROR("Failed to load test parameter. %s", error_desc.c_str());
				break;
			}

			Result exe_result;
			if (!LedgerManager::Instance().context_manager_.SyncTestProcess(LedgerContext::AT_TEST_V8,
				(TestParameter*)&test_parameter,
				utils::MICRO_UNITS_PER_SEC,
				exe_result, result["logs"], result["txs"], result["query_rets"], result["stat"])) {
				error_code = exe_result.code();
				error_desc = exe_result.desc();
				LOG_ERROR("Failed to execute the test.%s", error_desc.c_str());
				break;
			}
		} while (false);

		if (error_code == protocol::ERRCODE_CONTRACT_SYNTAX_ERROR) {
			reply_json["error_desc_json"].fromString(error_desc);
		}

		reply_json["error_code"] = error_code;
		reply_json["error_desc"] = error_desc;
		reply = reply_json.toStyledString();
	}

}