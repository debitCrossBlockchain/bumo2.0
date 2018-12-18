#include<cross/challenge_manager.h>
#include<cross/message_channel_manager.h>
#include <proto/cpp/overlay.pb.h>
using namespace std;
#include<map>
#define CHALLENGE_HEAD_SEQ "challenge_head_seq"
#define CHALLENGE_WITHDRAWAL_SEQ "challenge_withdrawal_seq"
#define MAX_REQUEST_SUBMIT_NUMS 10
namespace bumo {

	ChallengeSubmitHead::ChallengeSubmitHead() :
		chain_head_seq_(0),
		recv_max_seq_(0),
		latest_seq_(0){}

	ChallengeSubmitHead::~ChallengeSubmitHead(){}
	void ChallengeSubmitHead::UpdateSeq(){
		auto db = Storage::Instance().keyvalue_db();
		std::string str;
		Json::Value args;
		if (!db->Get(CHALLENGE_HEAD_SEQ, str)) {
			args["chain_seq"] = chain_head_seq_;
			db->Put(CHALLENGE_HEAD_SEQ, args.toFastString());
		}
		else{
			args.fromString(str.c_str());
			chain_head_seq_ = args["chain_seq"].asInt64();
		}
	}

	void ChallengeSubmitHead::UpdateSubmitHead(const protocol::MessageChannelSubmitHead &submit_head){
		utils::MutexGuard guard(submit_head_buffer_list_lock_);
		submit_head_buffer_list_.push_back(submit_head);
	}

	void ChallengeSubmitHead::CopyBufferSubmitHead(){
		std::list<protocol::MessageChannelSubmitHead> submit_head_list;
		{
			utils::MutexGuard guard(submit_head_buffer_list_lock_);
			submit_head_list.insert(submit_head_list.end(), submit_head_buffer_list_.begin(), submit_head_buffer_list_.end());
			submit_head_buffer_list_.clear();
		}

		utils::MutexGuard guard(common_lock_);
		std::list<protocol::MessageChannelSubmitHead>::const_iterator iter = submit_head_list.begin();
		while (iter != submit_head_list.end()){
			const protocol::MessageChannelSubmitHead &submit_head = *iter;
			if (submit_head.state() == -1){
				latest_seq_ = submit_head.header().seq();
			}
			ledger_map_.insert(pair<int64_t,protocol::LedgerHeader>(submit_head.header().seq(), submit_head.header()));
			iter++;
		}
	}

	void ChallengeSubmitHead::UpdateStatus(){
		utils::MutexGuard guard(common_lock_);
		if (ledger_map_.empty()){
			return;
		}

		UpdateRequestLatestSeq();

		//sort seq
		SortMap();

		//request
		RequestLost();
	}

	void ChallengeSubmitHead::SortMap(){
		//If cmc = chain max, ignore it
		if (latest_seq_ == recv_max_seq_){
			return;
		}
		for (auto itr = ledger_map_.begin(); itr != ledger_map_.end();){
			if (itr->second.seq() > latest_seq_){
				itr++;
				continue;
			}

			handlechallengeSubmitHead(itr->second);
			ledger_map_.erase(itr++);
		}
	}

	void ChallengeSubmitHead::handlechallengeSubmitHead(const protocol::LedgerHeader &header){
		if (header.seq() <= chain_head_seq_){
			return;
		}

		LedgerFrm frm;
		bool bflag = true;
		if (!frm.LoadFromDb(header.seq())) {
			std::string error_desc = utils::String::Format("Parse MessageChannelQueryHead error,no exist ledger_seq=(" FMT_I64 ")", header);
			LOG_ERROR("%s", error_desc.c_str());
			//TODO send CMC head challenge
		}

		const protocol::LedgerHeader& ledger_header = frm.GetProtoHeader();
		bflag = (ledger_header.chain_id() == header.chain_id()) && (ledger_header.account_tree_hash() == header.account_tree_hash()) && (ledger_header.seq() == header.seq()) &&
			(ledger_header.hash() == header.hash()) && (ledger_header.previous_hash()==header.previous_hash())&&(ledger_header.close_time()==header.close_time())&&
			(ledger_header.fees_hash()==header.fees_hash())&&(ledger_header.consensus_value_hash()==header.consensus_value_hash())&&(ledger_header.version()==header.version())&&
			(ledger_header.validators_hash()==header.validators_hash());
		if (!bflag){
			//TODO send CMC head challenge
		}

		int64_t max_seq = MAX(recv_max_seq_, header.seq());
		recv_max_seq_ = max_seq;
		chain_head_seq_ = max_seq;
		UpdateSeq();
	}

	void ChallengeSubmitHead::UpdateRequestLatestSeq(){
		protocol::MessageChannel message_channel;
		protocol::MessageChannelQuerySubmitHead query_head;
		query_head.set_seq(-1);
		query_head.set_hash("");
		query_head.set_chain_id(General::GetSelfChainId());
		message_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
		message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_QUWERY_SUBMIT_HEAD);
		message_channel.set_msg_data(query_head.SerializeAsString());
		bumo::MessageChannel::GetInstance()->MessageChannelProducer(message_channel);
	}

	void ChallengeSubmitHead::RequestLost(){
		//Request up to ten blocks
		int64_t max_nums = MIN(MAX_REQUEST_SUBMIT_NUMS, (recv_max_seq_ - latest_seq_));
		for (int64_t i = 1; i <= max_nums; i++){
			int64_t seq = latest_seq_ + i;
			auto itr = ledger_map_.find(seq);
			if (itr != ledger_map_.end()){
				continue;
			}

			std::string  error_desc;
			if (seq <= 0){
				std::string  error_desc = utils::String::Format("Parse MessageChannelQueryHead error,invalid ledger_seq(" FMT_I64 ")", seq);
				LOG_ERROR("%s", error_desc.c_str());
				return;
			}

			LedgerFrm frm;
			if (!frm.LoadFromDb(seq)) {
				error_desc = utils::String::Format("Parse MessageChannelQueryHead error,no exist ledger_seq=(" FMT_I64 ")", seq);
				LOG_ERROR("%s", error_desc.c_str());
				return;
			}
			const protocol::LedgerHeader& ledger_header = frm.GetProtoHeader();
			//Push message to child chain.
			protocol::MessageChannel message_channel;
			protocol::MessageChannelQuerySubmitHead query_head;
			query_head.set_seq(seq);
			query_head.set_hash(ledger_header.hash());
			message_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
			message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_QUWERY_SUBMIT_HEAD);
			message_channel.set_msg_data(query_head.SerializeAsString());
			bumo::MessageChannel::GetInstance()->MessageChannelProducer(message_channel);
		}
	}

	ChallengeWithdrawal::ChallengeWithdrawal() :
		chain_withdrawal_seq_(0),
		recv_max_seq_(0),
		latest_seq_(0){}

	ChallengeWithdrawal::~ChallengeWithdrawal(){}
	void ChallengeWithdrawal::InitSeq(){
		auto db = Storage::Instance().keyvalue_db();
		std::string str;
		Json::Value args;
		if (!db->Get(CHALLENGE_WITHDRAWAL_SEQ, str)) {
			args["chain_seq"] = chain_withdrawal_seq_;
			db->Put(CHALLENGE_WITHDRAWAL_SEQ, args.toFastString());
		}
		else{
			args.fromString(str.c_str());
			chain_withdrawal_seq_ = args["chain_seq"].asInt64();
		}
	}

	void ChallengeWithdrawal::UpdateStatus(){
		utils::MutexGuard guard(common_lock_);
		if (withdrawal_map_.empty()){
			return;
		}

		UpdateRequestLatestSeq();

		//sort seq
		SortMap();

		//request
		RequestLost();
	}

	void ChallengeWithdrawal::UpdateRequestLatestSeq(){
		protocol::MessageChannel message_channel;
		protocol::MessageChannelQueryWithdrawal withdrawal;
		withdrawal.set_seq(-1);
		withdrawal.set_chain_id(General::GetSelfChainId());
		message_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
		message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_QUWERY_WITHDRAWAL);
		message_channel.set_msg_data(withdrawal.SerializeAsString());
		bumo::MessageChannel::GetInstance()->MessageChannelProducer(message_channel);
	}

	void ChallengeWithdrawal::SortMap(){
		//If cmc = chain max, ignore it
		if (latest_seq_ == recv_max_seq_){
			return;
		}
		for (auto itr = ledger_map_.begin(); itr != ledger_map_.end();){
			if (itr->second.seq() > latest_seq_){
				itr++;
				continue;
			}

			handlechallengeSubmitHead(itr->second);
			ledger_map_.erase(itr++);
		}
	}

	void ChallengeWithdrawal::RequestLost(){
		//Request up to ten
		int64_t max_nums = MIN(MAX_REQUEST_SUBMIT_NUMS, (recv_max_seq_ - latest_seq_));
		for (int64_t i = 1; i <= max_nums; i++){
			int64_t seq = latest_seq_ + i;
			auto itr = withdrawal_map_.find(seq);
			if (itr != withdrawal_map_.end()){
				continue;
			}

			std::string  error_desc;
			if (seq <= 0){
				std::string  error_desc = utils::String::Format("Parse MessageChannelQueryWithdrawal error,invalid ledger_seq(" FMT_I64 ")", seq);
				LOG_ERROR("%s", error_desc.c_str());
				return;
			}
		
			protocol::MessageChannel message_channel;
			protocol::MessageChannelQueryWithdrawal withdrawal;
			withdrawal.set_seq(seq);
			withdrawal.set_chain_id(General::GetSelfChainId());
			message_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
			message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_QUWERY_WITHDRAWAL);
			message_channel.set_msg_data(withdrawal.SerializeAsString());
			bumo::MessageChannel::GetInstance()->MessageChannelProducer(message_channel);
		}
	}

	ChallengeManager::ChallengeManager() :
		enabled_(false),
		thread_ptr_(NULL){
		challenge_submit_head_ = std::make_shared<ChallengeSubmitHead>();
		challenge_withdrawal_ = std::make_shared<ChallengeWithdrawal>();
	}

	ChallengeManager::~ChallengeManager(){
		if (thread_ptr_){
			delete thread_ptr_;
			thread_ptr_ = NULL;
		}
	}

	void ChallengeManager::InitSeq(){
		challenge_submit_head_->UpdateSeq();
		challenge_withdrawal_->InitSeq();
	}

	bool ChallengeManager::Initialize() {
		enabled_ = true;
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("ChallengeManager")) {
			return false;
		}

		InitSeq();
		return true;
	}

	bool ChallengeManager::Exit(){
		enabled_ = false;
		if (thread_ptr_) {
			thread_ptr_->JoinWithStop();
		}
		return true;
	}

	void ChallengeManager::Run(utils::Thread *thread) {
		while (enabled_){
			utils::Sleep(10);
			int64_t current_time = utils::Timestamp::HighResolution();

		}
	}

	void ChallengeManager::ChallengeNotify(const protocol::MessageChannel &message_channel){
		switch (message_channel.msg_type()){
		case protocol::MESSAGE_CHANNEL_CHALLENGE_HEAD:{
														  HandleChallengeSubmitHead(message_channel);
														  break;
		}
		case protocol::MESSAGE_CHANNEL_CHALLENGE_WITHDRAWAL:{
																HandleChallengeWithdrawal(message_channel);
																break;
		}
		default:
			break;
		}
	}

	void ChallengeManager::HandleChallengeSubmitHead(const protocol::MessageChannel &message_channel){

	}

	void ChallengeManager::HandleChallengeWithdrawal(const protocol::MessageChannel &message_channel){

	}

}