#include "block_listen_manager.h"

namespace bumo {
	BlockListenManager::BlockListenManager(){
	}

	BlockListenManager::~BlockListenManager(){
	}

	bool BlockListenManager::Initialize(){
		return true;
	}

	bool BlockListenManager::Exit(){

		return true;
	}

	bool BlockListenManager::CommitBlock(){
		HandleMainChainBlock();
		HandleChildChainBlock();
		return true;
	}

	void BlockListenManager::HandleMainChainBlock(){
		//TODO: Handel child chain block, and call MessageChannel to send main chain proc //
	}

	void BlockListenManager::HandleChildChainBlock(){
		//TODO: Handel child chain block, and call MessageChannel to send main chain proc 
	}
}
