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
		latest_validates.clear();

		Json::Value input_value;
		input_value["method"] = "getValidators";

		Json::Value result_list;
		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_VALIDATOR_ADDRESS, input_value.toFastString(), result_list);
		if (error_code != protocol::ERRCODE_SUCCESS){
			LOG_ERROR("Failed to query validators .%d", error_code);
			return;
		}

		Json::Value object;
		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		object.fromString(result.c_str());

		if (!object["curValidators"].isArray()){
			LOG_ERROR("Failed to validators list is not array");
			return;
		}
		const Json::Value &curValidators = object["curValidators"];

		int32_t size = curValidators.size();
		for (int32_t i = 0; i < size; i++){
			std::string address = curValidators[i][Json::UInt(0)].asString().c_str();
			latest_validates.push_back(address);
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

	ChildValidatorProposer::ChildValidatorProposer() : ChildProposer(General::CONTRACT_VALIDATOR_ADDRESS){
		bumo::MessageChannel::Instance().RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_CHANGE_CHILD_VALIDATOR);
	}

	ChildValidatorProposer::~ChildValidatorProposer(){
		bumo::MessageChannel::Instance().UnregisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_CHANGE_CHILD_VALIDATOR);
	}

	int64_t ChildValidatorProposer::DoGetMessageIndex(const protocol::MessageChannel &message_channel){
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

	TransTask ChildValidatorProposer::DoBuildTxTask(const std::string &message_data){
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

	void ChildValidatorProposer::DoBuildRequestLostMessage(int64_t index, protocol::MessageChannel &message_channel){
		protocol::MessageChannelQueryChangeChildValidator query;
		query.set_chain_id(General::GetSelfChainId());
		query.set_change_child_index(index);
		message_channel.set_target_chain_id(0);
		message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_QUERY_CHANGE_CHILD_VALIDATOR);
		message_channel.set_msg_data(query.SerializeAsString());
	}

	bool ChildValidatorProposer::DoQueryProposalLatestIndex(int64_t &contract_latest_myself_index){
		Json::Value input_value;
		input_value["method"] = "queryProposal";
		Json::Value result_list;
		int32_t error_code = bumo::CrossUtils::QueryContract(target_address_, input_value.toFastString(), result_list);
		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		if (error_code != protocol::ERRCODE_SUCCESS || result.empty()){
			contract_latest_myself_index = -1;
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

	ChildProposerManager::ChildProposerManager(){
		validator_proposer_ = new ChildValidatorProposer();
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
		return true;
	}
	bool ChildProposerManager::Exit(){
		validator_proposer_->Exit();
		return true;
	}
}



