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

#include "main_proposer_manager.h"
#include <cross/cross_utils.h>
#include <ledger/ledger_manager.h>
#include <glue/glue_manager.h>
#include <overlay/peer_manager.h>
#include <algorithm>
namespace bumo {
	MainProposerManager::MainProposerManager(){
		for (int i = 0; i < MAX_CHAIN_ID; i++){
			child_chain_maps_[i].Reset();
		}

		bumo::MessageChannel::Instance().RegisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_SUBMIT_HEAD);
	}

	MainProposerManager::~MainProposerManager(){
		bumo::MessageChannel::Instance().UnregisterMessageChannelConsumer(this, protocol::MESSAGE_CHANNEL_SUBMIT_HEAD);
	}

	void MainProposerManager::DoTimerUpdate(){
		UpdateStatus();
		SendTransaction();
	}

	void MainProposerManager::UpdateStatus(){
		utils::MutexGuard guard(child_chain_map_lock_);
		for (int i = 0; i < MAX_CHAIN_ID; i++){
			ChildChain &child_chain = child_chain_maps_[i];
			if (child_chain.ledger_map.empty()){
				continue;
			}

			if (child_chain.error_tx_times > MAX_ERROR_TX_COUNT){
				BreakProposer("The proposal transaction went error MAX_ERROR_TX_COUNT times in a row");
			}

			//update latest validates 
			UpdateLatestValidates(i, child_chain.cmc_latest_validates);

			//If the node is not validate, all blocks are deleted
			bool is_validate = false;
			for (uint32_t j = 0; j < child_chain.cmc_latest_validates.size(); j++){
				if (source_address_ == child_chain.cmc_latest_validates[j]){
					is_validate = true;
				}
			}
			if (!is_validate){
				child_chain.Reset();
				continue;
			}

			//update latest child seq
			UpdateLatestSeq(i, child_chain.cmc_latest_seq);

			//sort seq
			SortChildSeq(child_chain);

			//request
			RequestChainSeq(child_chain);
		}
	}

	void MainProposerManager::UpdateTxResult(const int64_t &error_code, const std::string &error_desc, const std::string& hash){
		if (!enabled_){
			return;
		}

		utils::MutexGuard guard(child_chain_map_lock_);
		for (int64_t i = 0; i < MAX_CHAIN_ID; i++){
			ChildChain &child_chain = child_chain_maps_[i];
			//No data, ignore it
			if (child_chain.ledger_map.empty()){
				continue;
			}

			utils::StringList::const_iterator iter = std::find(child_chain.tx_history.begin(), child_chain.tx_history.end(), hash);
			if (iter == child_chain.tx_history.end()){
				continue;
			}

			if (error_code == protocol::ERRCODE_SUCCESS){
				child_chain.error_tx_times = 0;
				child_chain.tx_history.clear();
				continue;
			}
			++child_chain.error_tx_times;
			LOG_ERROR("Failed to Proposer Transaction,chain_id is(" FMT_I64 "),tx hash is %s,err_code is (" FMT_I64 "),err_desc is %s", 
				i, hash.c_str(), error_code, error_desc.c_str());
		}
	}

	void MainProposerManager::UpdateLatestValidates(const int64_t chain_id, utils::StringVector &latest_validates) const{
		latest_validates.clear();

		Json::Value input_value;
		Json::Value params;
		params["chain_id"] = chain_id;
		input_value["method"] = "queryChildChainValidators";
		input_value["params"] = params;

		Json::Value result_list;
		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input_value.toFastString(), result_list);

		Json::Value object;
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

		int32_t size = object["validators"].size();
		for (int32_t i = 0; i < size; i++){
			std::string address = object["validators"][i].asString().c_str();
			latest_validates.push_back(address);
		}
		return;
	}

	void MainProposerManager::UpdateLatestSeq(const int64_t chain_id, int64_t &seq) const{
		Json::Value params;
		params["chain_id"] = chain_id;
		params["header_hash"] = "";

		Json::Value input_value;
		input_value["method"] = "queryChildBlockHeader";
		input_value["params"] = params;

		Json::Value result_list;
		int32_t error_code = bumo::CrossUtils::QueryContract(General::CONTRACT_CMC_ADDRESS, input_value.toFastString(), result_list);
		std::string result = result_list[Json::UInt(0)]["result"]["value"].asString();
		Json::Value object;
		object.fromString(result.c_str());
		if (error_code != protocol::ERRCODE_SUCCESS){
			seq = -1;
			LOG_ERROR("Failed to query child block .%d", error_code);
			return;
		}

		seq = object["seq"].asInt64();
	}

	void MainProposerManager::SortChildSeq(ChildChain &child_chain) const{
		//Empty queues, ignore it
		if (child_chain.ledger_map.empty()){
			return;
		}

		//If cmc = chain max, ignore it
		if (child_chain.cmc_latest_seq == child_chain.recv_max_seq){
			return;
		}

		//Deletes invalid blocks
		LedgerMap &ledger_map = child_chain.ledger_map;
		for (auto itr = ledger_map.begin(); itr != ledger_map.end();){
			if (itr->second.seq() > child_chain.cmc_latest_seq){
				itr++; 
				continue;
			}

			ledger_map.erase(itr++);
		}
	}

	void MainProposerManager::RequestChainSeq(ChildChain &child_chain) const{
		LedgerMap &ledger_map = child_chain.ledger_map;
		//Request up to ten blocks
		int64_t max_nums = MIN(MAX_REQUEST_BLOCK_NUMS, (child_chain.recv_max_seq - child_chain.cmc_latest_seq));
		for (int64_t i = 1; i <= max_nums; i++){
			int64_t seq = child_chain.cmc_latest_seq + i;
			auto itr = ledger_map.find(seq);
			if (itr != ledger_map.end()){
				continue;
			}
			protocol::MessageChannel message_channel;
			protocol::MessageChannelQueryHead query_head;
			query_head.set_ledger_seq(seq);
			message_channel.set_target_chain_id(child_chain.chain_id);
			message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_QUERY_HEAD);
			message_channel.set_msg_data(query_head.SerializeAsString());
			bumo::MessageChannel::GetInstance()->MessageChannelProducer(message_channel);
		}
	}

	void MainProposerManager::SendTransaction(){
		utils::MutexGuard guard(child_chain_map_lock_);
		for (int64_t i = 0; i < MAX_CHAIN_ID; i++){
			ChildChain &child_chain = child_chain_maps_[i];
			//No data, ignore it
			if (child_chain.ledger_map.empty()){
				continue;
			}

			//Blocks are latest, ignore it
			if ((child_chain.recv_max_seq > 0) && child_chain.recv_max_seq <= child_chain.cmc_latest_seq){
				LOG_ERROR("recv_max_seq <= cmc_latest_seq, may be error");
				continue;
			}

			//Submit the latest five blocks
			std::vector<std::string> send_para_list;
			const LedgerMap &ledger_map = child_chain.ledger_map;
			for (int j = 1; j <= MAX_PACK_TX_COUNT; j++){
				LedgerMap::const_iterator itr = ledger_map.find(child_chain.cmc_latest_seq + j);
				if (itr == ledger_map.end()){
					break;
				}
				Json::Value block_header = bumo::Proto2Json(itr->second);
				Json::Value input_value;
				Json::Value params;

				params["chain_id"] = child_chain.chain_id;
				params["block_header"] = block_header;
				input_value["method"] = "submitChildBlockHeader";
				input_value["params"] = params;
				send_para_list.push_back(input_value.toFastString());
			}

			if (send_para_list.empty()){
				LOG_ERROR("send_para_list is empty, chain id:%d", i);
				return;
			}
			TransTask trans_task(send_para_list, 0, General::CONTRACT_CMC_ADDRESS, utils::String::ToString(i));
			TransactionSender::Instance().AsyncSendTransaction(this, trans_task);
		}
	}

	void MainProposerManager::DoHandleMessageChannel(const protocol::MessageChannel &message_channel){
		switch (message_channel.msg_type()){
		case protocol::MESSAGE_CHANNEL_SUBMIT_HEAD:
			OnHandleMainPropser(message_channel);
			break;
		case protocol::MESSAGE_CHANNEL_CHILD_CHALLENGE_HEAD:
			OnHandleChildChallengeSubmitHead(message_channel);
			break;
		case protocol::MESSAGE_CHANNEL_CHILD_CHALLENGE_WITHDRAWAL:
			OnHandleChildChallengeWithdrawal(message_channel);
			break;
		default:
			break;
		}
	}

	void MainProposerManager::OnHandleMainPropser(const protocol::MessageChannel &message_channel){
		if (message_channel.msg_type() != protocol::MESSAGE_CHANNEL_SUBMIT_HEAD){
			LOG_ERROR("Failed to message_channel type is not MESSAGE_CHANNEL_SUBMIT_HEAD, error msg type is %d", message_channel.msg_type());
			return;
		}

		protocol::LedgerHeader ledger_header;
		ledger_header.ParseFromString(message_channel.msg_data());

		if (ledger_header.chain_id() <= 0 || ledger_header.chain_id() >= MAX_CHAIN_ID){
			LOG_ERROR("chain id :(" FMT_I64 ")", ledger_header.chain_id());
			return;
		}

		utils::MutexGuard guard(child_chain_map_lock_);
		ChildChain &child_chain = child_chain_maps_[ledger_header.chain_id()];
		child_chain.ledger_map[ledger_header.seq()] = ledger_header;
		child_chain.recv_max_seq = MAX(child_chain.recv_max_seq, ledger_header.seq());
		child_chain.chain_id = ledger_header.chain_id();
	}

	void MainProposerManager::DoHandleSenderResult(const TransTask &task_task, const TransTaskResult &task_result){
		if (!utils::String::IsNumber(task_task.user_defined_)){
			BreakProposer("It's not a number");
			return;
		}

		int64_t chain_id = utils::String::Stoi64(task_task.user_defined_);
		if (chain_id < 0 || chain_id >= MAX_CHAIN_ID){
			BreakProposer(utils::String::Format("chain id :(" FMT_I64 ")", chain_id));
			return;
		}

		utils::MutexGuard guard(child_chain_map_lock_);
		child_chain_maps_[chain_id].tx_history.push_back(task_result.hash_);
	}

	void MainProposerManager::OnHandleChildChallengeSubmitHead(const protocol::MessageChannel &message_channel){
		if (General::GetSelfChainId() != General::MAIN_CHAIN_ID){
			return;
		}

		protocol::MessageChannelChildChallengeHead challenge_head;
		if (!challenge_head.ParseFromString(message_channel.msg_data())){
			int64_t error_code = protocol::ERRCODE_INVALID_PARAMETER;
			LOG_ERROR("Parse MessageChannelChildChallengeHead error, err_code is (" FMT_I64 ")", error_code);
			return;
		}

		Json::Value json_challenge = bumo::Proto2Json(challenge_head);
		std::vector<std::string> send_para_list;
	
		Json::Value input_value;
	
		input_value["method"] = "challengeSubmitHead";
		input_value["params"] = json_challenge;
		send_para_list.push_back(input_value.toFastString());
		TransTask trans_task(send_para_list, 1000000000, General::CONTRACT_CMC_ADDRESS, utils::String::ToString(challenge_head.chain_id()));
		TransactionSender::Instance().AsyncSendTransaction(this, trans_task);
	}


	void MainProposerManager::OnHandleChildChallengeWithdrawal(const protocol::MessageChannel &message_channel){

	}
}
