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

#include "proposer_manager.h"
#include<cross/cross_utils.h>
#include<ledger/ledger_manager.h>
#include <glue/glue_manager.h>
#include <overlay/peer_manager.h>
#include <algorithm>
namespace bumo {
	ProposerManager::ProposerManager() :
		enabled_(false),
		last_uptate_validate_address_time_(0),
		last_uptate_handle_child_chain_time_(0),
		last_uptate_child_chain_cashe_time_(0),
		last_uptate_handle_child_remove_time_(0),
		thread_ptr_(NULL){

	}

	ProposerManager::~ProposerManager(){
		if (thread_ptr_){
			delete thread_ptr_;
			thread_ptr_ = NULL;
		}
	}

	bool ProposerManager::Initialize(){
		if (General::GetSelfChainId() != General::MAIN_CHAIN_ID){
			return true;
		}

		enabled_ = true;
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("ProposerManager")) {
			return false;
		}
		bumo::MessageChannel::GetInstance()->RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_SUBMIT_HEAD);
		return true;
	}

	bool ProposerManager::Exit(){
		if (General::GetSelfChainId() != General::MAIN_CHAIN_ID){
			return true;
		}

		enabled_ = false;
		if (thread_ptr_) {
			thread_ptr_->JoinWithStop();
		}
		bumo::MessageChannel::GetInstance()->UnregisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_SUBMIT_HEAD);
		return true;
	}

	void ProposerManager::Run(utils::Thread *thread) {
		while (enabled_){
			int64_t current_time = utils::Timestamp::HighResolution();
			if (current_time > 2 * utils::MICRO_UNITS_PER_SEC + last_uptate_handle_child_chain_time_){
				//Handel block list//
				HandleChildChainBlock();
				last_uptate_handle_child_chain_time_ = current_time;
			}

			if (current_time > 5 * utils::MICRO_UNITS_PER_SEC + last_uptate_handle_child_remove_time_){
				//Handel block list//
				RemoveHandleChildChainBlock();
				last_uptate_handle_child_remove_time_ = current_time;
			}

			if (current_time > 2 * utils::MICRO_UNITS_PER_SEC + last_uptate_child_chain_cashe_time_){
				//Handel cashe list//
				HandleChildChainBlocklistCache();
				last_uptate_child_chain_cashe_time_ = current_time;
			}
		}
	}

	void ProposerManager::HandleSingleChildChainBlockNotExsit(const Header& header){
		protocol::MessageChannel message_channel;
		protocol::MessageChannelQueryHead query_head;
		Sleep(100);
		int64_t seq = 0;
		if (header.seq_ <= 0){
			seq = 1;
		}
		else{
			seq = header.seq_ + 1;
		}
		query_head.set_ledger_seq(seq);
		message_channel.set_target_chain_id(header.chanin_id_);
		message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_QUERY_HEAD);
		message_channel.set_msg_data(query_head.SerializeAsString());
		bumo::MessageChannel::GetInstance()->MessageChannelProducer(message_channel);
	}



	void ProposerManager::HandleChildChainBlock(){
		for (int64_t id = 1; id <= 10; id++){
			Sleep(100);
			HandleSingleChildChain(id);
		}
	}


	void ProposerManager::HandleSingleChildChain(int64_t chain_id){
		//handel child chain block, and call MessageChannel to send main chain proc 
		utils::MutexGuard guard(child_chain_list_lock_);
		protocol::LedgerHeader ledger_header;
		protocol::LedgerHeader ledger_header_seq;
		Header header;
		if (!QueryFreshChildBlock(chain_id, header)){
			return;
		}

		for (size_t seq = 0; seq < 10; seq++){
			ledger_header_seq.set_seq(header.seq_ + seq + 1);
			ledger_header_seq.set_chain_id(header.chanin_id_);
			std::list<protocol::LedgerHeader>::const_iterator iter_temp = std::find_if(handle_child_chain_block_list_.begin(), handle_child_chain_block_list_.end(), FindHeader(ledger_header_seq));
			if (iter_temp == handle_child_chain_block_list_.end()){
				Header header_temp;
				header_temp.seq_ = header.seq_ + seq;
				header_temp.chanin_id_ = header.chanin_id_;
				HandleSingleChildChainBlockNotExsit(header_temp);
				continue;
			}
		}

		std::list<protocol::LedgerHeader> ledger_header_list;
		for (size_t i = 0; i < 5; i++){
			ledger_header.set_seq(header.seq_ + i + 1);
			ledger_header.set_chain_id(header.chanin_id_);
			std::list<protocol::LedgerHeader>::const_iterator iter_temp = std::find_if(handle_child_chain_block_list_.begin(), handle_child_chain_block_list_.end(), FindHeader(ledger_header));
			if (iter_temp == handle_child_chain_block_list_.end()){
				break;
			}
			ledger_header_list.push_back(*iter_temp);
		}

		if (ledger_header_list.empty()){
			return;
		}

		PayCoinProposer(ledger_header_list);

	}

	void ProposerManager::HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel){
		if (message_channel.msg_type() != protocol::MESSAGE_CHANNEL_SUBMIT_HEAD){
			LOG_ERROR("Failed to message_channel type is not MESSAGE_CHANNEL_SUBMIT_HEAD, error msg type is %d", message_channel.msg_type());
			return;
		}

		protocol::LedgerHeader ledger_header;
		ledger_header.ParseFromString(message_channel.msg_data());
		AddChildChainBlocklistCache(ledger_header);
	}


	void ProposerManager::UpdateValidateAddressList(utils::StringList& validate_address, int64_t chain_id){

		Json::FastWriter json_input;
		Json::Value input_value;
		Json::Value params;
		params["chain_id"] = chain_id;
		input_value["method"] = "queryChildChainValidators";
		input_value["params"] = params;
		std::string input = json_input.write(input_value);


		Json::Value object;
		Json::Value result_list;

		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input.c_str(), result_list);

		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		object.fromString(result.c_str());

		if (error_code != protocol::ERRCODE_SUCCESS){
			LOG_ERROR("Failed to query validators .%d", error_code);
			return;
		}

		if (!object["validators"].isArray()){
			LOG_ERROR("Failed to validators list is not array");
			return;
		}

		validate_address.clear();
		int32_t size = object["validators"].size();
		for (int32_t i = 0; i < size; i++){
			std::string address = object["validators"][i].asString().c_str();
			validate_address.push_back(address.c_str());
		}

	}

	bool ProposerManager::CheckNodeIsValidate(int64_t chain_id){
		PrivateKey private_key(Configure::Instance().ledger_configure_.validation_privatekey_);
		std::string node_address = private_key.GetEncAddress();
		utils::StringList::const_iterator itor;
		bool flag = false;
		utils::StringList validate_address;
		UpdateValidateAddressList(validate_address, chain_id);
		itor = std::find(validate_address.begin(), validate_address.end(), node_address.c_str());
		if (itor != validate_address.end()){
			flag = true;
		}
		else{
			LOG_INFO("this node is not validators,address is %s,chain_id is %d", node_address.c_str(), chain_id);
			flag = false;
		}
		return flag;
	}

	bool ProposerManager::QueryFreshChildBlock(const int64_t chain_id, Header& header){

		bool flag = true;
		Json::FastWriter json_input;
		Json::Value input_value;
		Json::Value params;
		params["chain_id"] = chain_id;
		params["header_hash"] = "";
		input_value["method"] = "queryChildBlockHeader";
		input_value["params"] = params;
		std::string input = json_input.write(input_value);

		Json::Value object;
		Json::Value result_list;

		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input.c_str(), result_list);

		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		object.fromString(result.c_str());

		if (error_code != protocol::ERRCODE_SUCCESS){
			LOG_ERROR("Failed to query child block .%d", error_code);
			flag = false;
		}

		header.chanin_id_ = object["chain_id"].asInt64();
		header.seq_ = object["seq"].asInt64();
		return flag;
	}

	bool ProposerManager::CheckChildBlockExsit(const std::string& hash, int64_t chain_id){
		// Check for child chain block in CMC
		bool flag = false;
		Json::FastWriter json_input;
		Json::Value input_value;
		Json::Value params;
		params["chain_id"] = chain_id;
		params["header_hash"] = hash.c_str();
		input_value["method"] = "queryChildBlockHeader";
		input_value["params"] = params;
		std::string input = json_input.write(input_value);

		Json::Value object;
		Json::Value result_list;

		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input.c_str(), result_list);

		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		object.fromString(result.c_str());

		if (error_code != protocol::ERRCODE_SUCCESS){
			LOG_ERROR("Failed to query child block .%d", error_code);
			flag = false;
		}

		if (object["hash"].asString().compare(hash) == 0){
			LOG_INFO("child block is not exsit!");
			flag = true;
		}

		return flag;
	}


	void ProposerManager::RemoveHandleChildChainBlock(){
		utils::MutexGuard guard(child_chain_list_lock_);
		std::list<protocol::LedgerHeader>::const_iterator itor = handle_child_chain_block_list_.begin();
		while (itor != handle_child_chain_block_list_.end()){
			bool flag = false;
			Json::Value block_header = bumo::Proto2Json(*itor);
			flag = (!CheckNodeIsValidate(block_header["chain_id"].asInt64())) || CheckChildBlockExsit(block_header["hash"].asString(), block_header["chain_id"].asInt64());
			if (flag){
				handle_child_chain_block_list_.erase(itor++); // delete node£,find next node
			}
			else{
				++itor;
			}
		}

	}

	void ProposerManager::AddChildChainBlocklistCache(const protocol::LedgerHeader& ledger_header){
		utils::MutexGuard guard(child_chain_list_cashe_lock_);
		child_chain_block_list_cache_.push_back(ledger_header);
	}

	void ProposerManager::HandleChildChainBlocklistCache(){
		std::list<protocol::LedgerHeader> ledger_header_list;
		{
			utils::MutexGuard guard(child_chain_list_cashe_lock_);
			ledger_header_list.insert(ledger_header_list.end(), child_chain_block_list_cache_.begin(), child_chain_block_list_cache_.end());
			child_chain_block_list_cache_.clear();
		}

		utils::MutexGuard guard(child_chain_list_lock_);
		std::list<protocol::LedgerHeader>::const_iterator iter = ledger_header_list.begin();
		while (iter != ledger_header_list.end()){
			protocol::LedgerHeader ledger_header = *iter;
			std::list<protocol::LedgerHeader>::const_iterator iter_header = std::find_if(handle_child_chain_block_list_.begin(), handle_child_chain_block_list_.end(), FindHeader(ledger_header));
			if (iter_header == handle_child_chain_block_list_.end()){
				handle_child_chain_block_list_.push_back(ledger_header);
			}
			iter++;
		}
	}


	int32_t ProposerManager::PayCoinProposer(std::list<protocol::LedgerHeader> &ledger_header){

		PrivateKey private_key(Configure::Instance().ledger_configure_.validation_privatekey_);
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
			err_code = PayCoinSelf(Configure::Instance().ledger_configure_.validation_privatekey_, General::CONTRACT_CMC_ADDRESS, ledger_header, 0, nonce);
			nonce = nonce + 1;
			utils::Sleep(10);
		} while (err_code == protocol::ERRCODE_ALREADY_EXIST);

		return err_code;
	}

	int32_t ProposerManager::PayCoinSelf(const std::string &encode_private_key, const std::string &dest_address, std::list<protocol::LedgerHeader> &ledger_header, int64_t coin_amount, int64_t nonce){
		ledger_header.sort(std::bind(&bumo::ProposerManager::CompareHeader, this, std::placeholders::_1, std::placeholders::_2));
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

		std::list<protocol::LedgerHeader>::const_iterator iter = ledger_header.begin();
		while (iter != ledger_header.end()){
			protocol::Operation *ope = tran->add_operations();
			ope->set_type(protocol::Operation_Type_PAY_COIN);
			protocol::OperationPayCoin *pay_coin = ope->mutable_pay_coin();
			pay_coin->set_amount(coin_amount);
			pay_coin->set_dest_address(dest_address);

			Json::Value block_header = bumo::Proto2Json(*iter);
			Json::FastWriter json_input;
			Json::Value input_value;
			Json::Value params;

			params["chain_id"] = block_header["chain_id"].asInt64();
			params["block_header"] = block_header;
			input_value["method"] = "submitChildBlockHeader";
			input_value["params"] = params;
			std::string input = json_input.write(input_value);
			pay_coin->set_input(input);
			iter++;
		}

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
