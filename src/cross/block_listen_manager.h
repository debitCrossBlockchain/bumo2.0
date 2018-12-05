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

namespace bumo {
	class BlockListenManager :public utils::Singleton<BlockListenManager>,public ITransactionSenderNotify {
		friend class utils::Singleton<bumo::BlockListenManager>;
	public:
		BlockListenManager();
		~BlockListenManager(){}

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

		bool isMainChain_;
	};
}

#endif
