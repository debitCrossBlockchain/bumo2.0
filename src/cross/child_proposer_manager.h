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

#ifndef CHILD_VALIDATOR_MANAGER_H_
#define CHILD_VALIDATOR_MANAGER_H_

#include <utils/singleton.h>
#include <utils/thread.h>
#include <cross/cross_utils.h>
#include <cross/message_channel_manager.h>
#include <cross/base_proposer.h>
#include <cross/message_handler.h>

#define MAX_CHAIN_ID 1000
#define MAX_REQUEST_BLOCK_NUMS 100
#define MAX_ERROR_TX_COUNT 5
#define MAX_PACK_TX_COUNT 5

namespace bumo {
	class ChildProposer : public BaseProposer{
		typedef std::map<int64_t, std::string> ProposerMessageMap;

	public:
		ChildProposer(const std::string &target_address);
		virtual ~ChildProposer();
		void UpdateTxResult(const int64_t &error_code, const std::string &error_desc, const std::string& hash);

	private:
		virtual void DoTimerUpdate() override;
		virtual void DoHandleMessageChannel(const protocol::MessageChannel &message_channel) override;
		virtual void DoHandleSenderResult(const TransTask &task_task, const TransTaskResult &task_result) override;

		virtual TransTask DoBuildTxTask(const std::string &message_data) = 0;
		virtual int64_t DoGetMessageIndex(const protocol::MessageChannel &message_channel) = 0;
		virtual void DoBuildRequestLostMessage(int64_t index, protocol::MessageChannel &message_channel) = 0;
		virtual bool DoQueryProposalLatestIndex(int64_t &contract_latest_myself_index) = 0;

		void UpdateStatus();
		void SendTransaction();
		void UpdateLatestValidates(utils::StringVector &latest_validates) const;
		
		void SortMap(ProposerMessageMap &change_map) const;
		void RequestLost(ProposerMessageMap &change_map);
		
	protected:
		std::string target_address_;

	private:
		utils::Mutex common_lock_;
		ProposerMessageMap message_map_;
		utils::StringList tx_history_;

		int64_t error_tx_times_;
		int64_t contract_latest_myself_index_;
		int64_t recv_max_index_;
	};

	class ValidatorProposer : public ChildProposer{
	public:
		ValidatorProposer();
		~ValidatorProposer();

	private:
		virtual TransTask DoBuildTxTask(const std::string &message_data) override;
		virtual int64_t DoGetMessageIndex(const protocol::MessageChannel &message_channel) override;
		virtual void DoBuildRequestLostMessage(int64_t index, protocol::MessageChannel &message_channel) override;
		virtual bool DoQueryProposalLatestIndex(int64_t &contract_latest_myself_index);
	};

	class DepositProposer : public ChildProposer{
	public:
		DepositProposer();
		virtual ~DepositProposer();

	private:
		virtual TransTask DoBuildTxTask(const std::string &message_data) override;
		virtual int64_t DoGetMessageIndex(const protocol::MessageChannel &message_channel) override;
		virtual void DoBuildRequestLostMessage(int64_t index, protocol::MessageChannel &message_channel) override;
		virtual bool DoQueryProposalLatestIndex(int64_t &contract_latest_myself_index);
	};

	class MainChainAnswer : public bumo::IMessageChannelConsumer{
	public:
		MainChainAnswer();
		~MainChainAnswer();

	protected:
		virtual void HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel) override;

		void OnHandleQueryChangeValidator(const protocol::MessageChannel &message_channel);
		void OnHandleQueryDeposit(const protocol::MessageChannel &message_channel);

	private:
		bumo::MessageChannelPocMap proc_methods_;
	};

	class ChildProposerManager : public utils::Singleton<ChildProposerManager>{
		friend class utils::Singleton<bumo::ChildProposerManager>;
	public:
		ChildProposerManager();
		virtual ~ChildProposerManager();

		bool Initialize();
		bool Exit();
		void UpdateTxResult(const int64_t &error_code, const std::string &error_desc, const std::string& hash);

	private:
		std::shared_ptr<ValidatorProposer> validator_proposer_;
		std::shared_ptr<DepositProposer> deposit_proposer_;

		std::shared_ptr<MainChainAnswer> main_chain_answer_;
	};
}

#endif
