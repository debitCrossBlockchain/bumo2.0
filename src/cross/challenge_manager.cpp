#include<cross/challenge_manager.h>
# define CHALLENGE_SEQ "challenge_seq"
namespace bumo {

	ChallengeManager::ChallengeManager() :
		chain_seq_(0),
		enabled_(false),
		thread_ptr_(NULL){
	}

	ChallengeManager::~ChallengeManager(){
		if (thread_ptr_){
			delete thread_ptr_;
			thread_ptr_ = NULL;
		}
	}

	bool ChallengeManager::Initialize() {
		enabled_ = true;
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("ChallengeManager")) {
			return false;
		}

		auto db = Storage::Instance().keyvalue_db();
		std::string str;
		Json::Value args;
		if (!db->Get(CHALLENGE_SEQ, str)) {
			args["chain_seq"] = chain_seq_;
			db->Put(CHALLENGE_SEQ, args.toFastString());
		}
		else{
			args.fromString(str.c_str());
			chain_seq_ = args["chain_seq"].asInt64();
		}
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