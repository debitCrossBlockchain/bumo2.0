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

#ifndef BLOCK_LISTEN_MANAGER_H_
#define BLOCK_LISTEN_MANAGER_H_

#include <utils/singleton.h>
#include <utils/thread.h>
#include "ledger/ledger_frm.h"
#include <cross/cross_utils.h>

using namespace ::google::protobuf;
#define BUFFER_PERIOD 25
#define UPDATE_PERIOD 15
namespace bumo {
	class BlockListenManager :public utils::Singleton<BlockListenManager>, public ITransactionSenderNotify{
		friend class utils::Singleton<bumo::BlockListenManager>;
	public:
		BlockListenManager();
		~BlockListenManager();

		bool Initialize();
		bool Exit() { return true; }

		void HandleBlock(LedgerFrm::pointer closing_ledger);

	private:

		void HandleMainChainBlock(LedgerFrm::pointer closing_ledger);
		void HandleChildChainBlock(LedgerFrm::pointer closing_ledger);
		void DealTlog(LedgerFrm::pointer closing_ledger);

		std::string GetMerklePath(LedgerFrm::pointer closing_ledger);
		virtual void HandleTransactionSenderResult(const TransTask &task_task, const TransTaskResult &task_result) override;

		void SendChildHeader(LedgerFrm::pointer closing_ledger);
		void DealChildTlog(LedgerFrm::pointer closing_ledger, const protocol::Transaction &trans);
		void BuildSpvProof(LedgerFrm::pointer closing_ledger, const protocol::Transaction &trans, const protocol::OperationLog &tlog);


		void SendTlog(const protocol::OperationLog &tlog);
		void DealMainTlog(const protocol::Transaction &trans);


		void DealProposerTrans(const protocol::Transaction &trans, int64_t &error_code, const std::string &error_desc, const std::string& hash);


		protocol::MESSAGE_CHANNEL_TYPE BlockListenManager::ParseTlog(std::string tlog_topic);
		std::shared_ptr<Message> GetMsgObject(protocol::MESSAGE_CHANNEL_TYPE msg_type);
	private:

		bool isMainChain_;
	};

	class BlockListenBase :public utils::Runnable{
	public:
		BlockListenBase();
		virtual ~BlockListenBase();

		bool Initialize();
		bool Exit();
		virtual void HandleBlock(const LedgerFrm::pointer &closing_ledger);
		virtual void HandleTxEvent(const TransactionFrm::pointer &tx) = 0;
		virtual void HandleTlogEvent(const protocol::OperationLog &tlog) = 0;
	protected:
		virtual void Run(utils::Thread *thread) override;
		virtual protocol::MESSAGE_CHANNEL_TYPE ParseTlog(std::string tlog_topic);
		virtual void TlogToMessageChannel(const protocol::OperationLog &tlog);
	private:
		virtual void CopyBufferBlock() final;
		virtual void LedgerToTxs(const LedgerFrm::pointer &closing_ledger, std::list<protocol::Transaction> &tx_list) final;
		virtual void LedgerToTlogs(const LedgerFrm::pointer &closing_ledger, std::list<protocol::OperationLog> &tlog_list) final;
		virtual void BuildTx(const LedgerFrm::pointer &closing_ledger) final;
		virtual void BuildTlog(const LedgerFrm::pointer &closing_ledger) final;
		virtual void HandleBlockUpdate() final;
		virtual void MessageChannelToMsg(protocol::MESSAGE_CHANNEL_TYPE msg_type, std::shared_ptr<Message> &msg) final;
	private:
		bool enabled_;
		utils::Thread *thread_ptr_;
		utils::Mutex ledger_map_lock_;
		utils::Mutex ledger_buffer_list_lock_;
		std::map<int64, LedgerFrm::pointer> ledger_map_;
		std::list<LedgerFrm::pointer> ledger_buffer_list_;
		int64_t last_update_time_;
		int64_t last_buffer_time_;
	};

	class BlockListenMainChain :public BlockListenBase{
	public:
		BlockListenMainChain();
		virtual ~BlockListenMainChain();
		virtual void HandleTxEvent(const TransactionFrm::pointer &tx) override;
		virtual void HandleTlogEvent(const protocol::OperationLog &tlog) override;
		bool CheckTxTransaction(const protocol::Transaction &trans);
	};

}

#endif
