#include <cross/cross_manager.h>
#include<cross/block_listen_manager.h>
#include<cross/main_proposer_manager.h>
#include<cross/child_proposer_manager.h>
#include<cross/challenge_manager.h>
namespace bumo {

	CrossManager::CrossManager(){
		bumo::BlockListenManager::InitInstance();
		bumo::MainProposerManager::InitInstance();
		bumo::ChildProposerManager::InitInstance();
		bumo::ChallengeManager::InitInstance();	
	}

	CrossManager::~CrossManager(){
		bumo::BlockListenManager::ExitInstance();
		bumo::MainProposerManager::ExitInstance();
		bumo::ChildProposerManager::ExitInstance();
		bumo::ChallengeManager::ExitInstance();
	}

	bool  CrossManager::Initialize(){
		bumo::BlockListenManager &block_listen_handler = bumo::BlockListenManager::Instance();
		if (!block_listen_handler.Initialize()){
			LOG_ERROR_ERRNO("Failed to initialize child proposer");
			return false;
		}
	
		bumo::MainProposerManager &proposer = bumo::MainProposerManager::Instance();
		if (!proposer.Initialize(true)){
			LOG_ERROR_ERRNO("Failed to initialize block listen");
			return false;
		}
		
		bumo::ChildProposerManager &child_Proposer = bumo::ChildProposerManager::Instance();
		if (!child_Proposer.Initialize()){
			LOG_ERROR_ERRNO("Failed to initialize child proposer");
			return false;
		}

		bumo::ChallengeManager &challenge = bumo::ChallengeManager::Instance();
		if (!challenge.Initialize()){
			LOG_ERROR_ERRNO("Failed to initialize challenge");
			return false;
		}
		return true;
	}

	bool CrossManager::Exit(){
		bumo::BlockListenManager &block_listen_handler = bumo::BlockListenManager::Instance();
		if (!block_listen_handler.Exit()){
			LOG_ERROR_ERRNO("Failed to exit child proposer");
			return false;
		}

		bumo::MainProposerManager &proposer = bumo::MainProposerManager::Instance();
		if (!proposer.Exit()){
			LOG_ERROR_ERRNO("Failed to exit block listen");
			return false;
		}

		bumo::ChildProposerManager &child_Proposer = bumo::ChildProposerManager::Instance();
		if (!child_Proposer.Exit()){
			LOG_ERROR_ERRNO("Failed to exit child proposer");
			return false;
		}

		bumo::ChallengeManager &challenge = bumo::ChallengeManager::Instance();
		if (!challenge.Exit()){
			LOG_ERROR_ERRNO("Failed to exit challenge");
			return false;
		}
		return true;
	}

}
