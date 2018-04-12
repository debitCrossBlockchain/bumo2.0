#include "fee_compulate.h"

namespace bumo{

	const int64_t OperationGasConfigure::create_account = 0;
	const int64_t OperationGasConfigure::issue_asset = 5000000;
	const int64_t OperationGasConfigure::payment = 0;
	const int64_t OperationGasConfigure::set_metadata = 0;
	const int64_t OperationGasConfigure::set_sigure_weight = 0;
	const int64_t OperationGasConfigure::set_threshold = 0;
	const int64_t OperationGasConfigure::pay_coin = 0;
	const int64_t OperationGasConfigure::log = 1;

	int64_t FeeCompulate::ByteFee(const int64_t& price, const int64_t& tx_size){
		return price*tx_size;
	}

	int64_t FeeCompulate::OperationFee(const int64_t& price, const protocol::Operation_Type& op_type){
		int64_t fee = 0;
		switch (op_type) {
		case protocol::Operation_Type_UNKNOWN:
			break;
		case protocol::Operation_Type_CREATE_ACCOUNT:
			fee = OperationGasConfigure::create_account * price;
			break;
		case protocol::Operation_Type_PAYMENT:
			fee = OperationGasConfigure::payment * price;
			break;
		case protocol::Operation_Type_ISSUE_ASSET:
			fee = OperationGasConfigure::issue_asset * price;
			break;
		case protocol::Operation_Type_SET_METADATA:
			fee = OperationGasConfigure::set_metadata * price;
			break;
		case protocol::Operation_Type_SET_SIGNER_WEIGHT:
			fee = OperationGasConfigure::set_sigure_weight * price;
			break;
		case protocol::Operation_Type_SET_THRESHOLD:
			fee = OperationGasConfigure::set_threshold * price;
			break;
		case protocol::Operation_Type_PAY_COIN:
			fee = OperationGasConfigure::pay_coin * price;
			break;
		case protocol::Operation_Type_LOG:
			fee = OperationGasConfigure::log * price;
			break;
		default:
			fee = 0;
			break;
		}

		return fee;
	}

}