#include <cross/cross_manager.h>
#include<cross/message_channel_manager.h>
#include<cross/block_listen_manager.h>
#include<cross/main_proposer_manager.h>
#include<cross/child_proposer_manager.h>
#include<cross/challenge_manager.h>
namespace bumo {

	CrossManager::CrossManager(){
		bumo::MessageChannel::InitInstance();
		bumo::BlockListenManager::InitInstance();
		bumo::MainProposerManager::InitInstance();
		bumo::ChildProposerManager::InitInstance();
		bumo::ChallengeManager::InitInstance();	
	}

	CrossManager::~CrossManager(){
		bumo::MessageChannel::ExitInstance();
		bumo::BlockListenManager::ExitInstance();
		bumo::MainProposerManager::ExitInstance();
		bumo::ChildProposerManager::ExitInstance();
		bumo::ChallengeManager::ExitInstance();
	}

	bool  CrossManager::Initialize(){
		return true;
	}

	bool CrossManager::Exit(){
		return true;
	}

}
