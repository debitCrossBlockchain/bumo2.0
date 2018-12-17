#ifndef CHALLENGE_MANAGER_H_
#define CHALLENGE_MANAGER_H_

#include <utils/singleton.h>
#include <utils/thread.h>
#include "ledger/ledger_frm.h"
#include <cross/cross_utils.h>
namespace bumo {
	class ChallengeManager :public utils::Runnable{
	public:
		ChallengeManager();
		virtual ~ChallengeManager();
		bool Initialize();
		bool Exit();
		void ChallengeNotify(const protocol::MessageChannel &message_channel);
	private:
		void Run(utils::Thread *thread);
		void HandleChallengeSubmitHead(const protocol::MessageChannel &message_channel);
		void HandleChallengeWithdrawal(const protocol::MessageChannel &message_channel);
	private:
		bool enabled_;
		utils::Thread *thread_ptr_;
		int64_t chain_head_seq_;
		int64_t chain_withdrawal_seq_;
	};
}

#endif