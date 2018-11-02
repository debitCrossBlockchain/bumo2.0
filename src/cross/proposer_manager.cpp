#include "proposer_manager.h"

namespace bumo {
	ProposerManager::ProposerManager(){
		enabled_ = false;
		thread_ptr_ = NULL;
	}

	ProposerManager::~ProposerManager(){
		if (thread_ptr_){
			delete thread_ptr_;
			thread_ptr_ = NULL;
		}
	}

	bool ProposerManager::Initialize(){
		enabled_ = true;
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("ProposerManager")) {
			return false;
		}
		return true;
	}

	bool ProposerManager::Exit(){
		enabled_ = false;
		if (thread_ptr_) {
			thread_ptr_->JoinWithStop();
		}
		return true;
	}

	bool ProposerManager::CommitChildChainBlock(){
		//Push to a block map

		return true;
	}

	void ProposerManager::Run(utils::Thread *thread) {
		while (enabled_){
			//Handel block map//
			HandleChildChainBlock();
		}
	}

	void ProposerManager::HandleChildChainBlock(){
		//TODO: handel child chain block, and call MessageChannel to send main chain proc 

		if (CheckChildBlockExsit()){
			return;
		}
		
		CommitTransaction();
	}

	bool ProposerManager::CheckChildBlockExsit(){
		//TODO: Check for child chain block in CMC

		return true;
	}

	bool ProposerManager::CommitTransaction(){
		//TODO: create a mainchain transaction with private key to CMC

		return true;
	}
}
