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

#include <utils/crypto.h>
#include <common/storage.h>
#include <common/pb2json.h>
#include <main/configure.h>
#include <ledger/ledger_manager.h>
#include "transaction_frm.h"
#include "contract_manager.h"

#include "ledger_frm.h"
namespace bumo {

	TransactionFrm::TransactionFrm() :
		apply_time_(0),
		ledger_seq_(0),
		result_(),
		transaction_env_(),
		hash_(),
		full_hash_(),
		data_(),
		full_data_(),
		valid_signature_(),
		ledger_(),
		processing_operation_(0),
		real_fee_(0),
		max_end_time_(0),
		contract_step_(0),
		contract_memory_usage_(0),
		contract_stack_usage_(0),
		enable_check_(false), apply_start_time_(0), apply_use_time_(0),
		incoming_time_(utils::Timestamp::HighResolution()) {
		utils::AtomicInc(&bumo::General::tx_new_count);
	}


	TransactionFrm::TransactionFrm(const protocol::TransactionEnv &env) :
		apply_time_(0),
		ledger_seq_(0),
		result_(),
		transaction_env_(env),
		valid_signature_(),
		ledger_(),
		processing_operation_(0),
		real_fee_(0),
		max_end_time_(0),
		contract_step_(0),
		contract_memory_usage_(0),
		contract_stack_usage_(0),
		enable_check_(false), apply_start_time_(0), apply_use_time_(0),
		incoming_time_(utils::Timestamp::HighResolution()) {
		Initialize();
		utils::AtomicInc(&bumo::General::tx_new_count);
	}

	TransactionFrm::~TransactionFrm() {
		utils::AtomicInc(&bumo::General::tx_delete_count);
	}

	void TransactionFrm::ToJson(Json::Value &result) {
		result = Proto2Json(transaction_env_);
		result["error_code"] = result_.code();
		result["error_desc"] = result_.desc();
		result["close_time"] = apply_time_;
		result["ledger_seq"] = ledger_seq_;
		result["hash"] = utils::String::BinToHexString(hash_);
	}

	void TransactionFrm::Initialize() {
		const protocol::Transaction &tran = transaction_env_.transaction();
		data_ = tran.SerializeAsString();
		hash_ = HashWrapper::Crypto(data_);
		full_data_ = transaction_env_.SerializeAsString();
		full_hash_ = HashWrapper::Crypto(full_data_);

		for (int32_t i = 0; i < transaction_env_.signatures_size(); i++) {
			const protocol::Signature &signature = transaction_env_.signatures(i);
			PublicKey pubkey(signature.public_key());

			if (!pubkey.IsValid()) {
				LOG_ERROR("Invalid publickey(%s)", signature.public_key().c_str());
				continue;
			}
			if (!PublicKey::Verify(data_, signature.sign_data(), signature.public_key())) {
				LOG_ERROR("Invalid signature data(%s)", utils::String::BinToHexString(signature.SerializeAsString()).c_str());
				continue;
			}
			valid_signature_.insert(pubkey.GetEncAddress());
		}
	}

	std::string TransactionFrm::GetContentHash() const {
		return hash_;
	}

	std::string TransactionFrm::GetContentData() const {
		return data_;
	}

	std::string TransactionFrm::GetFullHash() const {
		return full_hash_;
	}

	const protocol::TransactionEnv &TransactionFrm::GetTransactionEnv() const {
		return transaction_env_;
	}

	std::string TransactionFrm::GetSourceAddress() const {
		const protocol::Transaction &tran = transaction_env_.transaction();
		return tran.source_address();
	}

	int64_t TransactionFrm::GetFee() const {
		return transaction_env_.transaction().fee();
	}

	int64_t TransactionFrm::GetSelfByteFee() {
		return LedgerManager::Instance().GetCurFeeConfig().byte_fee()*transaction_env_.ByteSize();
	}

	int64_t TransactionFrm::GetRealFee() const {
		return real_fee_;
	}

	void TransactionFrm::AddRealFee(int64_t fee) {
		real_fee_ += fee;
	}

	void TransactionFrm::SetApplyStartTime(int64_t time) {
		apply_start_time_ = time;
	}

	void TransactionFrm::SetApplyEndTime(int64_t time) {
		apply_use_time_ = time - apply_start_time_;
	}

	int64_t TransactionFrm::GetApplyTime() {
		return apply_use_time_;
	}

	void TransactionFrm::SetMaxEndTime(int64_t end_time) {
		max_end_time_ = end_time;
	}

	int64_t TransactionFrm::GetMaxEndTime() {
		return max_end_time_;
	}

	void TransactionFrm::ContractStepInc(int32_t step) {
		contract_step_ += step;
	}

	int32_t TransactionFrm::GetContractStep() {
		return contract_step_;
	}

	void TransactionFrm::SetMemoryUsage(int64_t memory_usage) {
		contract_memory_usage_ = memory_usage;
	}

	int64_t TransactionFrm::GetMemoryUsage() {
		return contract_memory_usage_;
	}

	void TransactionFrm::SetStackUsage(int64_t memory_usage) {
		contract_stack_usage_ = memory_usage;
	}

	int64_t TransactionFrm::GetStackUsage() {
		return contract_stack_usage_;
	}

	bool TransactionFrm::IsExpire(std::string &error_info) {
		if (!enable_check_) {
			return false;
		}

		if (contract_step_ > General::CONTRACT_STEP_LIMIT) {
			error_info = "Step expire";
			return true;
		}

		int64_t now = utils::Timestamp::HighResolution();
		if (max_end_time_ != 0 && now > max_end_time_) {
			error_info = "Time expire";
			return true;
		}

		if (contract_memory_usage_ > General::CONTRACT_MEMORY_LIMIT) {
			error_info = "Memory expire";
			return true;
		}

		if (contract_stack_usage_ > General::CONTRACT_STACK_LIMIT) {
			error_info = "Stack expire";
			return true;
		}

		return false;
	}

	void TransactionFrm::EnableChecked() {
		enable_check_ = true;
	}

	bool TransactionFrm::PayFee(std::shared_ptr<Environment> environment, int64_t &total_fee) {
		int64_t fee = GetFee();
		std::string str_address = transaction_env_.transaction().source_address();
		AccountFrm::pointer source_account;

		do {
			if (!environment->GetEntry(str_address, source_account)) {
				LOG_ERROR("Source account(%s) does not exists", str_address.c_str());
				result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
				break;
			}

			total_fee += fee;
			protocol::Account& proto_source_account = source_account->GetProtoAccount();
			int64_t new_balance = proto_source_account.balance() - fee;
			proto_source_account.set_balance(new_balance);

			return true;
		} while (false);

		return false;
	}

	int64_t TransactionFrm::GetNonce() const {
		return transaction_env_.transaction().nonce();
	}

	bool TransactionFrm::ValidForApply(std::shared_ptr<Environment> environment,bool check_priv) {
		do {
			if (!ValidForParameter())
				break;

			std::string str_address = transaction_env_.transaction().source_address();
			AccountFrm::pointer source_account;

			if (!environment->GetEntry(str_address, source_account)) {
				LOG_ERROR("Source account(%s) not exists", str_address.c_str());
				result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
				break;
			}

			//判断序号是否正确
			int64_t last_seq = source_account->GetAccountNonce();
			if (last_seq + 1 != GetNonce()) {
				LOG_ERROR("Account(%s) Tx sequence(" FMT_I64 ") not match reserve sequence (" FMT_I64 " + 1)",
					str_address.c_str(),
					GetNonce(),
					last_seq);
				result_.set_code(protocol::ERRCODE_BAD_SEQUENCE);
				break;
			}

			utils::StringVector vec;
			vec.push_back(transaction_env_.transaction().source_address());
			if (check_priv && !SignerHashPriv(source_account, -1)) {
				result_.set_code(protocol::ERRCODE_INVALID_SIGNATURE);
				result_.set_desc(utils::String::Format("Tx(%s) signatures not enough weight", utils::String::BinToHexString(hash_).c_str()));
				LOG_ERROR(result_.desc().c_str());
				break;
			}

			//first check fee for this transaction,but not include transaction triggered by contract
			int64_t bytes_fee = GetSelfByteFee();
			int64_t tran_fee = GetFee();
			if (LedgerManager::Instance().GetCurFeeConfig().byte_fee() > 0) {
				if (tran_fee < bytes_fee) {
					std::string error_desc = utils::String::Format(
						"Transaction(%s) fee(" FMT_I64 ") not enought for self byte fee(" FMT_I64 ") ",
						utils::String::BinToHexString(hash_).c_str(), tran_fee, bytes_fee);

					result_.set_code(protocol::ERRCODE_FEE_NOT_ENOUGH);
					result_.set_desc(error_desc);
					LOG_ERROR("%s", error_desc.c_str());
					return false;
				}

				if (source_account->GetAccountBalance() - tran_fee < LedgerManager::Instance().GetCurFeeConfig().base_reserve()) {
					std::string error_desc = utils::String::Format(
						"Account(%s) reserve balance not enough for transaction fee and base reserve: balance(" FMT_I64 ") - fee(" FMT_I64 ") < base_reserve(" FMT_I64 ")",
						str_address.c_str(), source_account->GetAccountBalance(), tran_fee, LedgerManager::Instance().GetCurFeeConfig().base_reserve());
					result_.set_code(protocol::ERRCODE_ACCOUNT_LOW_RESERVE);
					result_.set_desc(error_desc);
					LOG_ERROR("%s", error_desc.c_str());
					return false;
				}
			}
			return true;
		} while (false);

		return false;
	}

	bool TransactionFrm::CheckValid(int64_t last_seq) {
		AccountFrm::pointer source_account;
		if (!Environment::AccountFromDB(GetSourceAddress(), source_account)) {
			result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
			result_.set_desc(utils::String::Format("Source account(%s) not exist", GetSourceAddress().c_str()));
			LOG_ERROR("%s", result_.desc().c_str());
			return false;
		}

		int64_t bytes_fee = GetSelfByteFee();
		int64_t tran_fee = GetFee();
		if (tran_fee < 0){
			result_.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			result_.set_desc(utils::String::Format("Transaction(%s) fee(" FMT_I64 ") should not be negative number", utils::String::BinToHexString(hash_).c_str(), tran_fee));
			return false;
		}

		if (LedgerManager::Instance().GetCurFeeConfig().byte_fee() > 0) {
			if (tran_fee < bytes_fee) {
				std::string error_desc = utils::String::Format(
					"Transaction(%s) fee(" FMT_I64 ") not enough for self byte fee(" FMT_I64 ") ",
					utils::String::BinToHexString(hash_).c_str(), tran_fee, bytes_fee);

				result_.set_code(protocol::ERRCODE_FEE_NOT_ENOUGH);
				result_.set_desc(error_desc);
				LOG_ERROR("%s", error_desc.c_str());
				return false;
			}

			if (source_account->GetAccountBalance() - tran_fee < LedgerManager::Instance().GetCurFeeConfig().base_reserve()) {
				result_.set_code(protocol::ERRCODE_ACCOUNT_LOW_RESERVE);
				std::string error_desc = utils::String::Format("Account(%s) reserve balance not enough for transaction fee and base reserve: balance(" FMT_I64 ") - fee(" FMT_I64 ") < base reserve(" FMT_I64 "),last transaction hash(%s)",
					GetSourceAddress().c_str(), source_account->GetAccountBalance(), tran_fee, LedgerManager::Instance().GetCurFeeConfig().base_reserve(), utils::String::Bin4ToHexString(GetContentHash()).c_str());
				result_.set_desc(error_desc);
				LOG_ERROR("%s", error_desc.c_str());
				return false;
			}
		}

		if (GetNonce() <= source_account->GetAccountNonce()) {
			result_.set_code(protocol::ERRCODE_BAD_SEQUENCE);
			result_.set_desc(utils::String::Format("Tx nonce(" FMT_I64 ") too small, the account(%s) nonce is (" FMT_I64 ")",
				GetNonce(), GetSourceAddress().c_str(), source_account->GetAccountNonce()));
			LOG_ERROR("%s", result_.desc().c_str());
			return false;
		}

		if (!ValidForParameter())
			return false;

		if (last_seq == 0 && GetNonce() != source_account->GetAccountNonce() + 1) {

			result_.set_code(protocol::ERRCODE_BAD_SEQUENCE);
			result_.set_desc(utils::String::Format("Account(%s) tx sequence(" FMT_I64 ")  not match  reserve sequence (" FMT_I64 " + 1), txhash(%s)",
				GetSourceAddress().c_str(),
				GetNonce(),
				source_account->GetAccountNonce(),
				utils::String::Bin4ToHexString(GetContentHash()).c_str()));
			LOG_ERROR("%s", result_.desc().c_str());
			return false;
		}

		if (last_seq > 0 && (GetNonce() != last_seq + 1)) {
			result_.set_code(protocol::ERRCODE_BAD_SEQUENCE);
			result_.set_desc(utils::String::Format("Account(%s) Tx sequence(" FMT_I64 ")  not match  reserve sequence (" FMT_I64 " + 1)",
				GetSourceAddress().c_str(),
				GetNonce(),
				last_seq));
			LOG_ERROR("%s", result_.desc().c_str());
			return false;
		}
		return true;
	}

	bool TransactionFrm::ValidForParameter() {
		const protocol::Transaction &tran = transaction_env_.transaction();
		const LedgerConfigure &ledger_config = Configure::Instance().ledger_configure_;
		if (transaction_env_.ByteSize() >= General::TRANSACTION_LIMIT_SIZE) {
			LOG_ERROR("Transaction env size(%d) larger than limit(%d)",
				transaction_env_.ByteSize(),
				General::TRANSACTION_LIMIT_SIZE);
			result_.set_code(protocol::ERRCODE_TX_SIZE_TOO_BIG);
			return false;
		}

		bool check_valid = true;
		if (tran.operations_size() == 0) {
			LOG_ERROR("Operation size is zero");
			result_.set_code(protocol::ERRCODE_MISSING_OPERATIONS);
			result_.set_desc("Tx missing operation");
			check_valid = false;
			return check_valid;
		}

		if (tran.metadata().size() > General::METADATA_MAX_VALUE_SIZE) {
			result_.set_code(protocol::ERRCODE_INVALID_PARAMETER);
			//result_.set_desc("Transaction metadata too long");
			result_.set_desc(utils::String::Format("Length of the metadata from transaction exceeds the limit(%d).",
				General::METADATA_MAX_VALUE_SIZE));

			LOG_ERROR("%s", result_.desc().c_str());
			check_valid = false;
			return check_valid;
		}

		if (tran.expr_condition().size() > 0 &&
			!CheckExpr(tran.expr_condition(), utils::String::Format("Transaction(%s) ",
			utils::String::Bin4ToHexString(hash_).c_str()))) {
			check_valid = false;
			return check_valid;
		}

		check_valid = true;
		//判断operation的参数合法性
		int64_t t8 = utils::Timestamp::HighResolution();
		for (int i = 0; i < tran.operations_size(); i++) {
			protocol::Operation ope = tran.operations(i);
			std::string ope_source = !ope.source_address().empty() ? ope.source_address() : GetSourceAddress();
			if (!PublicKey::IsAddressValid(ope_source)) {
				check_valid = false;
				result_.set_code(protocol::ERRCODE_INVALID_ADDRESS);
				result_.set_desc("Source address not valid");
				LOG_ERROR("Invalid operation source address");
				break;
			}

			if (ope.metadata().size() > 0) {
				if (ope.metadata().size() > General::METADATA_MAX_VALUE_SIZE) {
					check_valid = false;
					result_.set_code(protocol::ERRCODE_INVALID_PARAMETER);
					result_.set_desc(utils::String::Format("Length of the metadata from operation(%d) exceeds the limit(%d).",
						i, General::METADATA_MAX_VALUE_SIZE));
					LOG_ERROR("%s", result_.desc().c_str());
					break;
				}
			}

			if (ope.expr_condition().size() > 0 &&
				!CheckExpr(ope.expr_condition(), utils::String::Format("Transaction(%s)'s Operation(id:%d) ",
				utils::String::Bin4ToHexString(hash_).c_str(), i))) {
				check_valid = false;
				break;
			}

			result_ = OperationFrm::CheckValid(ope, ope_source);

			if (result_.code() != protocol::ERRCODE_SUCCESS) {
				check_valid = false;
				break;
			}
		}
		return check_valid;
	}

	bool TransactionFrm::CheckExpr(const std::string &code, const std::string &log_prefix) {
		do {
			if (code.size() > General::EXPRCONDITION_MAXSIZE || code.size() == 0) {
				result_.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				result_.set_desc(utils::String::AppendFormat(code, "expression condition is too long or zero"));
				LOG_ERROR("%s", result_.desc().c_str());
				break;
			}

			protocol::ConsensusValue cons_null;
			ExprCondition expr(code, NULL, cons_null);
			utils::ExprValue value;
			result_ = expr.Parse(value);

			if (result_.code() != 0) {
				result_.set_code(protocol::ERRCODE_EXPR_CONDITION_SYNTAX_ERROR);
				result_.set_desc(utils::String::AppendFormat(log_prefix, "parse expression failed(%s)", result_.desc().c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				break;
			}

			if (value.type_ != utils::ExprValue::UNSURE && !value.IsSuccess()) {
				result_.set_code(protocol::ERRCODE_EXPR_CONDITION_RESULT_FALSE);
				result_.set_desc(utils::String::AppendFormat(log_prefix, "expression result predict false(%s)", value.Print().c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				break;
			}

			return true;
		} while (false);

		return false;
	}

	bool TransactionFrm::SignerHashPriv(AccountFrm::pointer account_ptr, int32_t type) const {
		const protocol::AccountPrivilege &priv = account_ptr->GetProtoAccount().priv();
		int64_t threshold = priv.thresholds().tx_threshold();
		int64_t type_threshold = account_ptr->GetTypeThreshold((protocol::Operation::Type)type);
		if (type_threshold > 0) {
			threshold = type_threshold;
		}

		if (valid_signature_.find(account_ptr->GetAccountAddress()) != valid_signature_.end()) {
			threshold -= priv.master_weight();
		}

		if (threshold <= 0) {
			return true;
		}

		for (int32_t i = 0; i < priv.signers_size(); i++) {
			const protocol::Signer &signer = priv.signers(i);

			if (valid_signature_.find(signer.address()) != valid_signature_.end()) {
				threshold -= signer.weight();
			}

			if (threshold <= 0) {
				return true;
			}
		}

		return false;
	}

	Result TransactionFrm::GetResult() const {
		return result_;
	}

	uint32_t TransactionFrm::LoadFromDb(const std::string &hash) {
		KeyValueDb *db = Storage::Instance().ledger_db();

		std::string txenv_store;
		int res = db->Get(ComposePrefix(General::TRANSACTION_PREFIX, hash), txenv_store);
		if (res < 0) {
			LOG_ERROR("Get transaction failed, %s", db->error_desc().c_str());
			return protocol::ERRCODE_INTERNAL_ERROR;
		}
		else if (res == 0) {
			LOG_ERROR("Tx(%s) not exist", utils::String::BinToHexString(hash).c_str());
			return protocol::ERRCODE_NOT_EXIST;
		}

		protocol::TransactionEnvStore envstor;
		if (!envstor.ParseFromString(txenv_store)) {
			LOG_ERROR("Decode tx(%s) body failed", utils::String::BinToHexString(hash).c_str());
			return protocol::ERRCODE_INTERNAL_ERROR;
		}

		apply_time_ = envstor.close_time();
		transaction_env_ = envstor.transaction_env();

		ledger_seq_ = envstor.ledger_seq();
		Initialize();
		result_.set_code(envstor.error_code());
		result_.set_desc(envstor.error_desc());
		return 0;
	}

	bool TransactionFrm::CheckTimeout(int64_t expire_time) {
		if (incoming_time_ < expire_time)
			return true;
		result_.set_code(protocol::ERRCODE_TX_TIMEOUT);
		return false;
	}

	bool TransactionFrm::ApplyExpr(const std::string &code, const std::string &log_prefix) {
		do {
			ExprCondition expr(code, environment_, ledger_->lpledger_context_->consensus_value_);
			utils::ExprValue value;
			result_ = expr.Eval(value);

			if (result_.code() != 0) {
				result_.set_code(protocol::ERRCODE_EXPR_CONDITION_SYNTAX_ERROR);
				result_.set_desc(utils::String::AppendFormat(log_prefix, "parse expression failed(%s)", result_.desc().c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				break;
			}

			if (!value.IsSuccess()) {
				result_.set_code(protocol::ERRCODE_EXPR_CONDITION_RESULT_FALSE);
				result_.set_desc(utils::String::AppendFormat(log_prefix, "expression result false(%s)", value.Print().c_str()));
				LOG_ERROR("%s", result_.desc().c_str());
				break;
			}

			return true;
		} while (false);

		return false;
	}

	void TransactionFrm::NonceIncrease(LedgerFrm* ledger_frm, std::shared_ptr<Environment> parent) {
		AccountFrm::pointer source_account;
		std::string str_address = GetSourceAddress();
		if (!parent->GetEntry(str_address, source_account)) {
			LOG_ERROR("Source account(%s) does not exists", str_address.c_str());
			result_.set_code(protocol::ERRCODE_ACCOUNT_NOT_EXIST);
			return;
		}
		source_account->NonceIncrease();
	}

	bool TransactionFrm::Apply(LedgerFrm* ledger_frm, std::shared_ptr<Environment> parent, bool bool_contract) {
		ledger_ = ledger_frm;

		if (parent->useAtomMap_)
			environment_ = parent;
		else
			environment_ = std::make_shared<Environment>(parent.get());

		bool bSucess = true;
		const protocol::Transaction &tran = transaction_env_.transaction();
		//check the expression
		if (tran.expr_condition().size() > 0 &&
			!ApplyExpr(tran.expr_condition(), utils::String::Format("Transaction(%s) ",
			utils::String::Bin4ToHexString(hash_).c_str()))) {
			bSucess = false;
			return bSucess;
		}

		std::shared_ptr<TransactionFrm> bottom_tx = ledger_frm->lpledger_context_->GetBottomTx();
		bottom_tx->AddRealFee(GetSelfByteFee());
		if (bottom_tx->GetRealFee() > bottom_tx->GetFee()) {
			result_.set_code(protocol::ERRCODE_FEE_NOT_ENOUGH);
			std::string error_desc = utils::String::Format("Transaction(%s) Fee(" FMT_I64 ") not enough,current real fee(" FMT_I64 "),Transaction(%s) self byte fee(" FMT_I64 ")",
				utils::String::BinToHexString(bottom_tx->GetContentHash()).c_str(), bottom_tx->GetFee(), bottom_tx->GetRealFee(),utils::String::BinToHexString(hash_).c_str(), GetSelfByteFee());
			result_.set_desc(error_desc);
			LOG_ERROR("%s", error_desc.c_str());
			bSucess = false;
			return bSucess;
		}

		for (processing_operation_ = 0; processing_operation_ < tran.operations_size(); processing_operation_++) {
			const protocol::Operation &ope = tran.operations(processing_operation_);
			std::shared_ptr<OperationFrm> opt = std::make_shared< OperationFrm>(ope, this, processing_operation_);
			if (opt == nullptr) {
				LOG_ERROR("Create operation frame failed");
				result_.set_code(protocol::ERRCODE_INVALID_PARAMETER);
				bSucess = false;
				break;
			}

			if (!bool_contract && !ledger_->IsTestMode()) {
				if (!opt->CheckSignature(environment_)) {
					LOG_ERROR("Check signature operation frame failed, txhash(%s)", utils::String::Bin4ToHexString(GetContentHash()).c_str());
					result_ = opt->GetResult();
					bSucess = false;
					break;
				}
			}

			//check the expression
			if (ope.expr_condition().size() > 0 &&
				!ApplyExpr(ope.expr_condition(), utils::String::Format("Transaction(%s)'s Operation(id:%d) ",
				utils::String::Bin4ToHexString(hash_).c_str(), processing_operation_))) {
				bSucess = false;
				break;
			}

			//opt->SourceRelationTx();
			Result result = opt->Apply(environment_);

			if (result.code() != 0) {
				result_ = opt->GetResult();
				bSucess = false;
				LOG_ERROR_ERRNO("Transaction(%s) operation(%d) apply failed",
					utils::String::BinToHexString(hash_).c_str(), processing_operation_, result_.code(), result_.desc().c_str());
				break;
			}

			bottom_tx->AddRealFee(opt->GetOpeFee());
			if (bottom_tx->GetRealFee() > bottom_tx->GetFee()) {
				result_.set_code(protocol::ERRCODE_FEE_NOT_ENOUGH);
				std::string error_desc = utils::String::Format("Transaction(%s) Fee(" FMT_I64 ") not enough,current real fee(" FMT_I64 "), Transaction(%s) operation(%d) fee(" FMT_I64 ")",
					utils::String::BinToHexString(bottom_tx->GetContentHash()).c_str(), bottom_tx->GetFee(), bottom_tx->GetRealFee(),utils::String::BinToHexString(hash_).c_str(), processing_operation_, opt->GetOpeFee());
				result_.set_desc(error_desc);
				LOG_ERROR("%s", error_desc.c_str());
				bSucess = false;
				break;
			}
		}

		return bSucess;
	}
}


