#include<cross/challenge_manager.h>
namespace bumo {

	ChallengeManager::ChallengeManager() :
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
		if (!thread_ptr_->Start("BlockListenManager")) {
			return false;
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

}