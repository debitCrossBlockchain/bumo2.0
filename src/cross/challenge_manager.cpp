#include<cross/challenge_manager.h>
#include<cross/message_channel_manager.h>
#define CHALLENGE_HEAD_SEQ "challenge_head_seq"
#define CHALLENGE_WITHDRAWAL_SEQ "challenge_withdrawal_seq"
#define MAX_REQUEST_SUBMIT_NUMS 100
namespace bumo {

	ChallengeSubmitHead::ChallengeSubmitHead() :
		chain_head_seq_(0),
		recv_max_seq_(0),
		latest_seq_(0){}

	ChallengeSubmitHead::~ChallengeSubmitHead(){}
	void ChallengeSubmitHead::InitSeq(){
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

	void ChallengeSubmitHead::UpdateStatus(){
		utils::MutexGuard guard(common_lock_);
		if (ledger_map_.empty()){
				return;
			}

			
			//update latest child seq
			//UpdateLatestSeq(i,latest_seq);

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

			ledger_map_.erase(itr++);
		}
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
			protocol::MessageChannel message_channel;
			protocol::MessageChannelQueryHead query_head;
			query_head.set_ledger_seq(seq);
			message_channel.set_target_chain_id(General::MAIN_CHAIN_ID);
			message_channel.set_msg_type(protocol::MESSAGE_CHANNEL_QUERY_HEAD);
			message_channel.set_msg_data(query_head.SerializeAsString());
			bumo::MessageChannel::GetInstance()->MessageChannelProducer(message_channel);
		}
	}

	ChallengeWithdrawal::ChallengeWithdrawal() :
		chain_withdrawal_seq_(0),
		recv_max_seq(0),
		latest_seq(0){}

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
		challenge_submit_head_->InitSeq();
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