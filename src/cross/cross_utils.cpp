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

#include <glue/glue_manager.h>
#include <overlay/peer_manager.h>
#include<ledger/fee_calculate.h>
#include <utils/base_int.h>

#include "cross_utils.h"

namespace bumo {

	int32_t CrossUtils::QueryContract(const std::string &address, const std::string &input, Json::Value &query_rets){
		std::string result = "";
		ContractTestParameter parameter;
		parameter.code_ = "";
		parameter.input_ = input;
		parameter.opt_type_ = ContractTestParameter::QUERY;
		parameter.contract_address_ = address;
		parameter.source_address_ = "";
		parameter.fee_limit_ = 1000000000000;
		parameter.gas_price_ = LedgerManager::Instance().GetCurFeeConfig().gas_price();
		parameter.contract_balance_ = 1000000000000;

		int32_t error_code = protocol::ERRCODE_SUCCESS;
		AccountFrm::pointer acc = NULL;
		utils::Sleep(10);
		do {
			if (parameter.contract_address_.empty()) {
				error_code = protocol::ERRCODE_INVALID_PARAMETER;
				result = "Empty contract address";
				LOG_ERROR("%s", result.c_str());
				break;
			}

			if (!Environment::AccountFromDB(parameter.contract_address_, acc)) {
				error_code = protocol::ERRCODE_NOT_EXIST;
				result = utils::String::Format("Account(%s) is not existed", parameter.contract_address_.c_str());
				LOG_ERROR("Failed to load the account from the database. %s", result.c_str());
				break;
			}

			parameter.code_ = acc->GetProtoAccount().contract().payload();

			if (parameter.code_.empty()) {
				error_code = protocol::ERRCODE_NOT_EXIST;
				result = utils::String::Format("Account(%s) has no contract code", parameter.contract_address_.c_str());
				LOG_ERROR("Failed to load test parameter. %s", result.c_str());
				break;
			}

			Result exe_result;
			Json::Value result_json = Json::Value(Json::objectValue);
			if (!LedgerManager::Instance().context_manager_.SyncTestProcess(LedgerContext::AT_TEST_V8,
				(TestParameter*)&parameter,
				utils::MICRO_UNITS_PER_SEC,
				exe_result, result_json["logs"], result_json["txs"], result_json["query_rets"], result_json["stat"])) {
				error_code = exe_result.code();
				result = exe_result.desc();
				LOG_ERROR("Failed to execute the test.%s", result.c_str());
				break;
			}

			query_rets = result_json["query_rets"];
		} while (false);

		//LOG_INFO("Query result code:%d, result:%s", error_code, result.c_str());

		return error_code;
	}

	int32_t CrossUtils::SendTransaction(TransactionFrm::pointer tran_ptr) {
		Result result;
		GlueManager::Instance().OnTransaction(tran_ptr, result);
		if (result.code() != 0) {
			LOG_ERROR("Pay coin result code:%d, des:%s", result.code(), result.desc().c_str());
			return result.code();
		}

		PeerManager::Instance().Broadcast(protocol::OVERLAY_MSGTYPE_TRANSACTION, tran_ptr->GetProtoTxEnv().SerializeAsString());
		return protocol::ERRCODE_SUCCESS;
	}

	TransactionFrm::pointer CrossUtils::BuildTransaction(const std::string &private_key, const std::string &dest, const std::vector<std::string> &paras, int64_t nonce, int64_t fee_limit){
		PrivateKey pkey(private_key);
		if (!pkey.IsValid()){
			LOG_ERROR("Private key is not valid");
			return nullptr;
		}
		
		protocol::TransactionEnv tran_env;
		protocol::Transaction *tran = tran_env.mutable_transaction();
		tran->set_source_address(pkey.GetEncAddress());
		tran->set_nonce(nonce);
		for (unsigned i = 0; i < paras.size(); i++){
			protocol::Operation *ope = tran->add_operations();
			ope->set_type(protocol::Operation_Type_PAY_COIN);
			protocol::OperationPayCoin *pay_coin = ope->mutable_pay_coin();
			pay_coin->set_amount(0);
			pay_coin->set_dest_address(dest);
			pay_coin->set_input(paras[i]);
		}

		tran->set_gas_price(LedgerManager::Instance().GetCurFeeConfig().gas_price());
		tran->set_fee_limit(0);

		if (fee_limit <= 0){
			fee_limit = 0;
			//300 is signature byte
			if (!utils::SafeIntMul(tran->gas_price(), ((int64_t)tran_env.ByteSize() + 300), fee_limit)){
				LOG_ERROR("Failed to evaluate fee.");
				return nullptr;
			}
		}
		tran->set_fee_limit(fee_limit);

		std::string content = tran->SerializeAsString();
		std::string sign = pkey.Sign(content);
		protocol::Signature *signpro = tran_env.add_signatures();
		signpro->set_sign_data(sign);
		signpro->set_public_key(pkey.GetEncPublicKey());

		TransactionFrm::pointer ptr = std::make_shared<TransactionFrm>(tran_env);
		return ptr;
	}
}