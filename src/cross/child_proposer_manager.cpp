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
#include<ledger/ledger_manager.h>
#include <glue/glue_manager.h>
#include <overlay/peer_manager.h>
#include <cross/child_proposer_manager.h>

namespace bumo {
	ChildProposer::ChildProposer(const std::string &target_address){
		error_tx_times_ = 0;
		contract_latest_myself_index_ = 0;
		target_address_ = target_address;
	}

	ChildProposer::~ChildProposer(){
	}

	void ChildProposer::DoTimerUpdate(){
		UpdateStatus();
		SendTransaction();
	}

	void ChildProposer::UpdateStatus(){
		utils::MutexGuard guard(common_lock_);

		if (error_tx_times_ > MAX_ERROR_TX_COUNT){
			BreakProposer("The child validator transaction went error MAX_ERROR_TX_COUNT times in a row");
		}

		//update latest validates 
		utils::StringVector latest_validates;
		UpdateLatestValidates(latest_validates);

		//If the node is not validate, all blocks are deleted
		bool is_validate = false;
		for (uint32_t j = 0; j < latest_validates.size(); j++){
			if (source_address_ == latest_validates[j]){
				is_validate = true;
			}
		}
		if (!is_validate){
			message_map_.clear();
			return;
		}

		//update latest child seq
		if (!DoQueryProposalLatestIndex(contract_latest_myself_index_)){
			return;
		}

		//sort seq
		SortMap(message_map_);

		//request
		RequestLost(message_map_);
	}

	void ChildProposer::UpdateTxResult(const int64_t &error_code, const std::string &error_desc, const std::string& hash){
		if (!enabled_){
			return;
		}

		utils::MutexGuard guard(common_lock_);
		//No data, ignore it
		if (message_map_.empty()){
			return;
		}

		utils::StringList::const_iterator iter = std::find(tx_history_.begin(), tx_history_.end(), hash);
		if (iter == tx_history_.end()){
			return;
		}

		if (error_code == protocol::ERRCODE_SUCCESS){
			error_tx_times_ = 0;
			tx_history_.clear();
			return;
		}
		error_tx_times_++;
		LOG_ERROR("Failed to Child Validator Transaction, (" FMT_I64 "),tx hash is %s,err_code is (" FMT_I64 "),err_desc is %s", 
			hash.c_str(), error_code, error_desc.c_str());
	}

	void ChildProposer::UpdateLatestValidates(utils::StringVector &latest_validates) const{
		auto sets = LedgerManager::Instance().Validators();

		for (int i = 0; i < sets.validators_size(); i++){
			auto validator = sets.mutable_validators(i);
			latest_validates.push_back(validator->address());
		}

		return;
	}

	void ChildProposer::SortMap(ProposerMessageMap &change_map) const{
		//Empty queues, ignore it
		if (change_map.empty()){
			return;
		}

		//If contract == recv max, ignore it
		if (contract_latest_myself_index_ == recv_max_index_){
			return;
		}

		//Deletes invalid message
		for (auto itr = change_map.begin(); itr != change_map.end();){
			if (itr->first > contract_latest_myself_index_){
				itr++; 
				continue;
			}
			change_map.erase(itr++);
		}
	}

	void ChildProposer::RequestLost(ProposerMessageMap &change_map){
		//Request up to ten blocks
		int64_t max_nums = MIN(MAX_REQUEST_BLOCK_NUMS, (recv_max_index_ - contract_latest_myself_index_));
		if (max_nums <= 0){
			max_nums = 1;
		}
		for (int64_t i = 1; i <= max_nums; i++){
			int64_t index = contract_latest_myself_index_ + i;
			auto itr = change_map.find(index);
			if (itr != change_map.end()){
				continue;
			}
			protocol::MessageChannel message_channel;
			DoBuildRequestLostMessage(index, message_channel);
			
			bumo::MessageChannel::Instance().MessageChannelProducer(message_channel);
		}
	}

	void ChildProposer::SendTransaction(){
		utils::MutexGuard guard(common_lock_);
		//No data, ignore it
		if (message_map_.empty()){
			return;
		}

		//Blocks are latest, ignore it
		if ((recv_max_index_ > 0) && recv_max_index_ <= contract_latest_myself_index_){
			LOG_TRACE("recv_max_seq <= cmc_latest_seq, may be ignore it");
			return;
		}

		ProposerMessageMap::const_iterator itr = message_map_.find(contract_latest_myself_index_ + 1);
		if (itr == message_map_.end()){
			return;
		}

		TransTask trans_task = DoBuildTxTask(itr->second);
		TransactionSender::Instance().AsyncSendTransaction(this, trans_task);
	}

	void ChildProposer::DoHandleMessageChannel(const protocol::MessageChannel &message_channel){
		int64_t index = DoGetMessageIndex(message_channel);
		if (index <= 0){
			return;
		}

		utils::MutexGuard guard(common_lock_);
		message_map_[index] = message_channel.msg_data();
		recv_max_index_ = MAX(recv_max_index_, index);
	}

	void ChildProposer::DoHandleSenderResult(const TransTask &task_task, const TransTaskResult &task_result){
		utils::MutexGuard guard(common_lock_);
		tx_history_.push_back(task_result.hash_);
	}

	ValidatorProposer::ValidatorProposer() : ChildProposer(General::CONTRACT_VALIDATOR_ADDRESS){
		bumo::MessageChannel::Instance().RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_CHANGE_CHILD_VALIDATOR);
	}

	ValidatorProposer::~ValidatorProposer(){
		bumo::MessageChannel::Instance().UnregisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_CHANGE_CHILD_VALIDATOR);
	}

	int64_t ValidatorProposer::DoGetMessageIndex(const protocol::MessageChannel &message_channel){
		if (message_channel.msg_type() != protocol::MESSAGE_CHANNEL_CHANGE_CHILD_VALIDATOR){
			LOG_ERROR("Failed to message_channel type is not MESSAGE_CHANNEL_CHANGE_CHILD_VALIDATOR, error msg type is %d", message_channel.msg_type());
			return -1;
		}

		protocol::MessageChannelChangeChildValidator change;
		change.ParseFromString(message_channel.msg_data());

		if (change.main_chain_tx_hash().empty()){
			return -1;
		}

		if (change.add_validator().empty() && change.delete_validator().empty()){
			return -1;
		}
		return utils::String::Stoi64(change.index());
	}

	TransTask ValidatorProposer::DoBuildTxTask(const std::string &message_data){
		protocol::MessageChannelChangeChildValidator change;
		change.ParseFromString(message_data);

		Json::Value change_data = bumo::Proto2Json(change);
		Json::Value input_value;

		input_value["method"] = "changeValidator";
		input_value["params"] = change_data;

		std::vector<std::string> send_para_list;
		send_para_list.push_back(input_value.toFastString());
		TransTask trans_task(send_para_list, 0, target_address_, "");
		return trans_task;
	}

	void ValidatorProposer::DoBuildRequestLostMessage(int64_t index, protocol::MessageChannel &message_channel){
		protocol::MessageChannelQueryChangeChildValidator query;
		query.set_chain_id(General::GetSelfChainId());
		query.set_change_child_index(index);

		message_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
		message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_QUERY_CHANGE_CHILD_VALIDATOR);
		message_channel.set_msg_data(query.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(message_channel);
	}

	bool ValidatorProposer::DoQueryProposalLatestIndex(int64_t &contract_latest_myself_index){
		Json::Value input_value;
		input_value["method"] = "queryProposal";
		Json::Value result_list;
		int32_t error_code = bumo::CrossUtils::QueryContract(target_address_, input_value.toFastString(), result_list);
		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		if (error_code != protocol::ERRCODE_SUCCESS || result.empty()){
			contract_latest_myself_index = 0;
			LOG_ERROR("Failed to query child block.%d, result:%s", error_code, result.c_str());
			return false;
		}

		Json::Value object;
		object.fromString(result.c_str());
		const Json::Value &proposal = object["proposal"];
		if (proposal.isBool() && !proposal.asBool()){
			LOG_TRACE("Failed to query proposal.");
			return true;
		}

		contract_latest_myself_index = proposal["index"].asInt64();
		if (proposal["executed"].asBool()){
			return true;
		}

		const Json::Value &validators = proposal["validator"];
		for (uint32_t i = 0; i < validators.size(); i++){
			if (validators[i][Json::UInt(0)].asString() == source_address_){
				return true;
			}
		}

		//If you do not submit, contract_latest_myself_index - 1
		contract_latest_myself_index = contract_latest_myself_index - 1;
		return true;
	}

	DepositProposer::DepositProposer() : ChildProposer(General::CONTRACT_CPC_ADDRESS){
		bumo::MessageChannel::Instance().RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_DEPOSIT);
	}

	DepositProposer::~DepositProposer(){
		bumo::MessageChannel::Instance().UnregisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_DEPOSIT);
	}

	TransTask DepositProposer::DoBuildTxTask(const std::string &message_data){
		protocol::MessageChannelDeposit deposit;
		deposit.ParseFromString(message_data);

		std::vector<std::string> send_para_list;
		//Json::Value deposit_data = bumo::Proto2Json(deposit);
		Json::Value input_value;
		Json::Value params;

		params["chain_id"] = deposit.chain_id();
		params["seq"] = utils::String::ToString(deposit.seq());
		params["deposit_data"]["address"] = deposit.address();
		params["deposit_data"]["amount"] = utils::String::ToString(deposit.amount());

		input_value["method"] = "deposit";
		input_value["params"] = params;
		send_para_list.push_back(input_value.toFastString());

		TransTask trans_task(send_para_list, 0, target_address_, "");
		return trans_task;
	}

	int64_t DepositProposer::DoGetMessageIndex(const protocol::MessageChannel &message_channel){
		if (message_channel.msg_type() != protocol::MESSAGE_CHANNEL_DEPOSIT){
			LOG_ERROR("Failed to message_channel type is not MESSAGE_CHANNEL_DEPOSIT, error msg type is %d", message_channel.msg_type());
			return -1;
		}

		protocol::MessageChannelDeposit deposit;
		deposit.ParseFromString(message_channel.msg_data());
		if (deposit.chain_id() != General::GetSelfChainId() || 
			deposit.amount() <= 0 || deposit.address().empty() || 
			deposit.block_number() <= 0 || deposit.seq() <= 0){
			LOG_ERROR("Failed to parse message, error msg type is %d", message_channel.msg_type());
			return -1;
		}

		return deposit.seq();
	}

	void DepositProposer::DoBuildRequestLostMessage(int64_t index, protocol::MessageChannel &message_channel){
		protocol::MessageChannelQueryDeposit query_deposit;
		query_deposit.set_chain_id(General::GetSelfChainId());
		query_deposit.set_seq(index);
		message_channel.set_target_chain_id(0);
		message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_QUERY_DEPOSIT);
		message_channel.set_msg_data(query_deposit.SerializeAsString());
	}

	bool DepositProposer::DoQueryProposalLatestIndex(int64_t &contract_latest_myself_index){
		Json::Value result_list;
		Json::Value input_value;
		input_value["method"] = "queryChildDeposit";
		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_CPC_ADDRESS, input_value.toFastString(), result_list);
		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		if (error_code != protocol::ERRCODE_SUCCESS || result.empty()){
			contract_latest_myself_index = 0;
			LOG_ERROR("Failed to query childChainDeposit seq .%d", error_code);
			return false;
		}
		
		Json::Value object;
		object.fromString(result.c_str());
		int64_t index = utils::String::Stoi64(object["index"].asString());
		if (index <= 0){
			LOG_TRACE("Failed to query proposal.");
			return true;
		}

		contract_latest_myself_index = index;
		if (object["executed"].asInt() > 0){
			return true;
		}

		const Json::Value &validators = object["validators"];
		for (uint32_t i = 0; i < validators.size(); i++){
			if (validators[i].asString() == source_address_){
				return true;
			}
		}

		//If you do not submit, contract_latest_myself_index - 1
		contract_latest_myself_index = contract_latest_myself_index - 1;
		return true;
	}


	MainChainAnswer::MainChainAnswer(){
		MessageChannel::Instance().RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_QUERY_CHANGE_CHILD_VALIDATOR);
		MessageChannel::Instance().RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_QUERY_DEPOSIT);
		proc_methods_[protocol::MESSAGE_CHANNEL_QUERY_CHANGE_CHILD_VALIDATOR] = std::bind(&MainChainAnswer::OnHandleQueryChangeValidator, this, std::placeholders::_1);
		proc_methods_[protocol::MESSAGE_CHANNEL_QUERY_DEPOSIT] = std::bind(&MainChainAnswer::OnHandleQueryDeposit, this, std::placeholders::_1);
	}

	MainChainAnswer::~MainChainAnswer(){

	}

	void MainChainAnswer::HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel){
		MessageChannelPoc proc;
		MessageChannelPocMap::iterator iter = proc_methods_.find(message_channel.msg_type());
		if (iter == proc_methods_.end()) {
			LOG_TRACE("Type(" FMT_I64 ") not found", message_channel.msg_type());
			return;
		}

		iter->second(message_channel);
	}

	void MainChainAnswer::OnHandleQueryChangeValidator(const protocol::MessageChannel &message_channel){
		protocol::MessageChannelQueryChangeChildValidator query;
		protocol::ERRORCODE error_code = protocol::ERRCODE_SUCCESS;
		std::string error_desc = "";
		int64_t chain_id = 0;
		int64_t index = 0;

		if (!query.ParseFromString(message_channel.msg_data())){
			error_desc = utils::String::Format("Parse MessageChannelQueryHead error!");
			error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}
		chain_id = query.chain_id();
		index = query.change_child_index();
		if (chain_id <= 0 || index <= 0){
			error_desc = utils::String::Format("Parse OnHandleQueryChangeValidator error,invalid chain_id(" FMT_I64 ")", chain_id);
			error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}


		Json::Value query_rets;
		Json::Value input_value;
		Json::Value params;
		input_value["method"] = "queryChangeValidatorHistory";
		input_value["params"]["chainId"] = chain_id;
		input_value["params"]["index"] = index;
		if (protocol::ERRCODE_SUCCESS != bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input_value.toFastString(), query_rets)){
			error_desc = utils::String::Format("Query contract error!%s", query_rets.toFastString().c_str());
			error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}

		if (!query_rets.isArray()){
			error_desc = utils::String::Format("Query contract error! Json result is not array.%s", query_rets.toFastString().c_str());
			error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}

		Json::Value custom_result;
		custom_result.fromString(query_rets[Json::UInt(0)]["result"]["value"].asString());
		std::string error_msg;
		protocol::MessageChannelChangeChildValidator change_child_validator;
		if (!Json2Proto(custom_result, change_child_validator, error_msg)){
			error_desc = utils::String::Format("Invalid contract result:%s", custom_result.toFastString().c_str());
			error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("%s", error_desc.c_str());
			return;
		}

		//Push message to child chain.
		protocol::MessageChannel msg_channel;
		msg_channel.set_target_chain_id(chain_id);
		msg_channel.set_msg_type(protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_CHANGE_CHILD_VALIDATOR);
		msg_channel.set_msg_data(change_child_validator.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(msg_channel);
	}

	void MainChainAnswer::OnHandleQueryDeposit(const protocol::MessageChannel &message_channel){
		protocol::MessageChannelQueryDeposit query_deposit;
		if (General::GetSelfChainId() != General::MAIN_CHAIN_ID){
			return;
		}

		if (!query_deposit.ParseFromString(message_channel.msg_data())){
			int64_t error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("Parse MessageChannelQueryDeposit error, err_code is (" FMT_I64 ")", error_code);
			return;
		}

		Json::Value params;
		Json::Value input_value;
		if (query_deposit.seq() == -1){
			params["chain_id"] = query_deposit.chain_id();
			input_value["method"] = "queryChildFreshDeposit";
		}
		else{
			params["chain_id"] = query_deposit.chain_id();
			params["seq"] = query_deposit.seq();
			input_value["method"] = "queryChildDeposit";
		}

		input_value["params"] = params;

		Json::Value result_list;
		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input_value.toFastString(), result_list);
		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		Json::Value object;
		object.fromString(result.c_str());
		if (error_code != protocol::ERRCODE_SUCCESS){
			LOG_ERROR("Failed to query child deposit .%d", error_code);
			return;
		}

		protocol::MessageChannelDeposit deposit;
		protocol::MessageChannel msg_channel;
		deposit.set_chain_id(object["chain_id"].asInt64());
		deposit.set_amount(object["amount"].asInt64());
		deposit.set_seq(object["seq"].asInt64());
		deposit.set_block_number(object["block_number"].asInt64());
		deposit.set_source_address(object["source_address"].asString());
		deposit.set_address(object["address"].asString());

		msg_channel.set_target_chain_id(query_deposit.chain_id());
		msg_channel.set_msg_type(protocol::MESSAGE_CHANNEL_TYPE::MESSAGE_CHANNEL_DEPOSIT);
		msg_channel.set_msg_data(deposit.SerializeAsString());
		MessageChannel::Instance().MessageChannelProducer(msg_channel);
	}

	ChildProposerManager::ChildProposerManager(){
		validator_proposer_ = std::make_shared<ValidatorProposer>();
		deposit_proposer_ = std::make_shared<DepositProposer>();
		main_chain_answer_ = std::make_shared<MainChainAnswer>();
	}
	ChildProposerManager::~ChildProposerManager(){
	}

	void ChildProposerManager::UpdateTxResult(const int64_t &error_code, const std::string &error_desc, const std::string& hash){
		if (validator_proposer_){
			validator_proposer_->UpdateTxResult(error_code, error_desc, hash);
		}
	}

	bool ChildProposerManager::Initialize(){
		bool is_child = (General::GetSelfChainId() != General::MAIN_CHAIN_ID);
		validator_proposer_->Initialize(is_child);
		deposit_proposer_->Initialize(is_child);
		return true;
	}
	bool ChildProposerManager::Exit(){
		validator_proposer_->Exit();
		deposit_proposer_->Exit();
		return true;
	}
}



