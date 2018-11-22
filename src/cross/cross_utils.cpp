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
#include "cross_utils.h"

namespace bumo {

	bool CrossUtils::EvaluateFee(protocol::TransactionEnv &tran_env, Result& result, int64_t& max, int64_t& min){
		protocol::Transaction *tran = tran_env.mutable_transaction();
		int64_t pay_amount = 0;
		std::string tx_source_address = tran->source_address();
		AccountFrm::pointer source_account;
		if (!Environment::AccountFromDB(tx_source_address, source_account)) {
			result.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
			result.set_desc(utils::String::Format("Source account(%s) not exist", tx_source_address.c_str()));
			LOG_ERROR("Failed to load the account from the database.%s", result.desc().c_str());
			return false;
		}

		int64_t total_opt_fee = 0;
		for (int i = 0; i < tran->operations_size(); i++) {
			const protocol::Operation &ope = tran->operations(i);
			std::string ope_source_address = ope.source_address();
			if (ope_source_address.size() == 0)
				ope_source_address = tx_source_address;
			if (tx_source_address == ope_source_address){
				auto type = ope.type();
				int64_t opt_price;
				if (!utils::SafeIntMul(FeeCalculate::GetOperationTypeGas(ope), tran->gas_price(), opt_price)) {
					result.set_code(protocol::ERRCODE_MATH_OVERFLOW);
					result.set_desc(utils::String::Format("Source account(%s) math overflow, GetOperationTypeGas:(" FMT_I64 "), gas_price:(" FMT_I64 ")",
						tx_source_address.c_str(), FeeCalculate::GetOperationTypeGas(ope), tran->gas_price()));
					LOG_ERROR("Failed to evaluate fee.%s", result.desc().c_str());
					return false;
				}

				if (!utils::SafeIntAdd(total_opt_fee, opt_price, total_opt_fee)){
					result.set_code(protocol::ERRCODE_MATH_OVERFLOW);
					result.set_desc(utils::String::Format("Source account(%s) math overflow, total_opt_fee:(" FMT_I64 "), opt_price:(" FMT_I64 ")",
						tx_source_address.c_str(), total_opt_fee, opt_price));
					LOG_ERROR("Failed to evaluate fee.%s", result.desc().c_str());
					return false;
				}

				if (type == protocol::Operation_Type_PAY_COIN) {
					if (!utils::SafeIntAdd(pay_amount, ope.pay_coin().amount(), pay_amount)) {
						result.set_code(protocol::ERRCODE_MATH_OVERFLOW);
						result.set_desc(utils::String::Format("Source account(%s) math overflow, pay_amount:(" FMT_I64 "), pay_coin().amount:(" FMT_I64 ")",
							tx_source_address.c_str(), pay_amount, ope.pay_coin().amount()));
						LOG_ERROR("Failed to evaluate fee.%s", result.desc().c_str());
						return false;
					}
				}
				if (type == protocol::Operation_Type_CREATE_ACCOUNT) {
					if (!utils::SafeIntAdd(pay_amount, ope.create_account().init_balance(), pay_amount)) {
						result.set_code(protocol::ERRCODE_MATH_OVERFLOW);
						result.set_desc(utils::String::Format("Source account(%s) math overflow, pay_amount:(" FMT_I64 "), init_balance:(" FMT_I64 ")",
							tx_source_address.c_str(), pay_amount, ope.create_account().init_balance()));
						LOG_ERROR("Failed to evaluate fee.%s", result.desc().c_str());
						return false;
					}
				}
			}
		}

		int64_t balance = source_account->GetAccountBalance();
		int64_t fee;
		if (!utils::SafeIntSub(balance, LedgerManager::Instance().GetCurFeeConfig().base_reserve(), fee)) {
			result.set_code(protocol::ERRCODE_MATH_OVERFLOW);
			result.set_desc(utils::String::Format("Source account(%s) overflow for fee, balance:(" FMT_I64 "), base_reserve:(" FMT_I64 "), pay_amount:(" FMT_I64 ")",
				tx_source_address.c_str(), balance, LedgerManager::Instance().GetCurFeeConfig().base_reserve(), pay_amount));
			LOG_ERROR("Failed to evaluate fee.%s", result.desc().c_str());
			return false;
		}

		if (!utils::SafeIntSub(fee, pay_amount, fee)) {
			result.set_code(protocol::ERRCODE_MATH_OVERFLOW);
			result.set_desc(utils::String::Format("Source account(%s) overflow for fee, balance:(" FMT_I64 "), base_reserve:(" FMT_I64 "), pay_amount:(" FMT_I64 ")",
				tx_source_address.c_str(), balance, LedgerManager::Instance().GetCurFeeConfig().base_reserve(), pay_amount));
			LOG_ERROR("Failed to evaluate fee.%s", result.desc().c_str());
			return false;
		}
		tran->set_fee_limit(fee);

		int64_t bytes_fee = 0;
		if (tran->gas_price() > 0) {
			if (!utils::SafeIntMul(tran->gas_price(), (int64_t)tran_env.ByteSize(), bytes_fee)){
				result.set_code(protocol::ERRCODE_MATH_OVERFLOW);
				result.set_desc(utils::String::Format("Source account(%s) overflow for fee, gas_price:(" FMT_I64 "), ByteSize:%d",
					tx_source_address.c_str(), tran->gas_price(), tran->ByteSize()));
				LOG_ERROR("Failed to evaluate fee.%s", result.desc().c_str());
				return false;
			}
		}

		int64_t total_fee;
		if (!utils::SafeIntAdd(bytes_fee, total_opt_fee, total_fee)) {
			result.set_code(protocol::ERRCODE_MATH_OVERFLOW);
			result.set_desc(utils::String::Format("Source account(%s) overflow for fee, bytes_fee:(" FMT_I64 "), total_opt_fee:(" FMT_I64 ")",
				tx_source_address.c_str(), bytes_fee, total_opt_fee));
			LOG_ERROR("Failed to evaluate fee.%s", result.desc().c_str());
			return false;
		}

		if (fee < total_fee) {
			result.set_code(protocol::ERRCODE_FEE_NOT_ENOUGH);
			result.set_desc(utils::String::Format("Source account(%s) not enough balance for fee", tx_source_address.c_str()));
			LOG_ERROR("Failed to evaluate fee.%s", result.desc().c_str());
			return false;
		}

		min = total_fee;
		max = fee;
		return true;
	}

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

	int32_t CrossUtils::PayCoin(const std::string &encode_private_key, const std::string &dest_address, const std::string &contract_input, int64_t coin_amount) {
		PrivateKey private_key(encode_private_key);
		if (!private_key.IsValid()){
			LOG_ERROR("Private key is not valid");
			return protocol::ERRCODE_INVALID_PRIKEY;
		}
		int32_t err_code = 0;
		int64_t nonce = 0;
		std::string source_address = private_key.GetEncAddress();

		AccountFrm::pointer account_ptr;
		if (!Environment::AccountFromDB(source_address, account_ptr)) {
			LOG_ERROR("Address:%s not exsit", source_address.c_str());
			return protocol::ERRCODE_INVALID_PRIKEY;
		}

		nonce = account_ptr->GetAccountNonce() + 1;

		do {
			CrossUtils obj;
			err_code = obj.PayCoinSelf(encode_private_key, dest_address, contract_input, coin_amount, nonce);
			nonce = nonce + 1;
			utils::Sleep(10);
		} while (err_code == protocol::ERRCODE_ALREADY_EXIST);

		return err_code;
	}

	int32_t CrossUtils::PayCoinSelf(const std::string &encode_private_key, const std::string &dest_address, const std::string &contract_input, int64_t coin_amount, int64_t nonce){

		PrivateKey private_key(encode_private_key);
		if (!private_key.IsValid()){
			LOG_ERROR("Private key is not valid");
			return protocol::ERRCODE_INVALID_PRIKEY;
		}

		std::string source_address = private_key.GetEncAddress();

		protocol::TransactionEnv tran_env;
		protocol::Transaction *tran = tran_env.mutable_transaction();
		tran->set_source_address(source_address);
		tran->set_fee_limit(100000000);
		tran->set_gas_price(LedgerManager::Instance().GetCurFeeConfig().gas_price());
		tran->set_nonce(nonce);
		protocol::Operation *ope = tran->add_operations();
		ope->set_type(protocol::Operation_Type_PAY_COIN);
		protocol::OperationPayCoin *pay_coin = ope->mutable_pay_coin();
		pay_coin->set_amount(coin_amount);
		pay_coin->set_dest_address(dest_address);
		pay_coin->set_input(contract_input);

		std::string content = tran->SerializeAsString();
		std::string sign = private_key.Sign(content);
		protocol::Signature *signpro = tran_env.add_signatures();
		signpro->set_sign_data(sign);
		signpro->set_public_key(private_key.GetEncPublicKey());

		Result result;
		TransactionFrm::pointer ptr = std::make_shared<TransactionFrm>(tran_env);
		GlueManager::Instance().OnTransaction(ptr, result);
		if (result.code() != 0) {
			LOG_ERROR("Pay coin result code:%d, des:%s", result.code(), result.desc().c_str());
			return result.code();
		}

		PeerManager::Instance().Broadcast(protocol::OVERLAY_MSGTYPE_TRANSACTION, tran_env.SerializeAsString());
		std::string tx_hash = utils::String::BinToHexString(HashWrapper::Crypto(content)).c_str();
		LOG_INFO("Pay coin tx hash %s", tx_hash.c_str());
		return protocol::ERRCODE_SUCCESS;
	}
}