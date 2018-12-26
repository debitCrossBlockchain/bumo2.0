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
	class BlockListenBase :public utils::Runnable{
	public:
		BlockListenBase();
		virtual ~BlockListenBase();

		bool Initialize();
		bool Exit();
		virtual void HandleBlock(const LedgerFrm::pointer &closing_ledger);
		virtual void HandleBlockEvent(const LedgerFrm::pointer &closing_ledger) = 0;
		virtual void HandleTxEvent(const TransactionFrm::pointer &tx) = 0;
		virtual void HandleTlogEvent(const LedgerFrm::pointer &closing_ledger,const protocol::OperationLog &tlog) = 0;
	protected:
		virtual void Run(utils::Thread *thread) override;
		virtual protocol::MESSAGE_CHANNEL_TYPE ParseTlog(std::string tlog_topic);
		virtual void TlogToMessageChannel(const protocol::OperationLog &tlog);
	private:
		virtual void TxFrmToTlog(const LedgerFrm::pointer &closing_ledger, const TransactionFrm::pointer &txFrm);
		virtual void CopyBufferBlock() final;
		virtual void BuildTx(const LedgerFrm::pointer &closing_ledger) final;
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
		virtual void HandleBlockEvent(const LedgerFrm::pointer &closing_ledger) override;
		virtual void HandleTxEvent(const TransactionFrm::pointer &tx) override;
		virtual void HandleTlogEvent(const LedgerFrm::pointer &closing_ledger,const protocol::OperationLog &tlog);
		bool CheckTxTransaction(const protocol::Transaction &trans);
	};

	class BlockListenChildChain :public BlockListenBase{
	public:
		BlockListenChildChain();
		virtual ~BlockListenChildChain();
		virtual void HandleBlockEvent(const LedgerFrm::pointer &closing_ledger) override;
		virtual void HandleTxEvent(const TransactionFrm::pointer &tx) override;
		virtual void HandleTlogEvent(const LedgerFrm::pointer &closing_ledger,const protocol::OperationLog &tlog);
		
	private:
		virtual void TlogToMessageChannelWithdrawal(const LedgerFrm::pointer &closing_ledger, const protocol::OperationLog &tlog) final;
		virtual void HandleChildHeader(LedgerFrm::pointer closing_ledger);
		
	};

	class BlockListenManager :public utils::Singleton<BlockListenManager>{
		friend class utils::Singleton<bumo::BlockListenManager>;
	public:
		BlockListenManager();
		~BlockListenManager();

		bool Initialize();
		bool Exit();

		void HandleBlock(LedgerFrm::pointer closing_ledger);
	private:
		std::shared_ptr<BlockListenMainChain> block_listen_main_chain_;
		std::shared_ptr<BlockListenChildChain> block_listen_child_chain_;
	};
}

#endif
