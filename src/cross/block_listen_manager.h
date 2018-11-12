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


namespace bumo {
	class BlockListenManager :public utils::Singleton<BlockListenManager>{
		friend class utils::Singleton<bumo::BlockListenManager>;
	public:
		BlockListenManager();
		~BlockListenManager();

		bool Initialize();
		bool Exit();
		//bool CommitBlock();
		bool HandleBlock(LedgerFrm::pointer closing_ledger);

	private:
		void HandleMainChainBlock(LedgerFrm::pointer closing_ledger);
		void HandleChildChainBlock(LedgerFrm::pointer closing_ledger);

		//MainChain have Tx'Msg which need to transfer to ChildChain
		const protocol::OperationLog * PickTransferTlog(TransactionFrm::pointer txFrm);

		protocol::MESSAGE_CHANNEL_TYPE BlockListenManager::FilterTlog(std::string tlog_topic);
		::google::protobuf::Message * GetMsgObject(protocol::MESSAGE_CHANNEL_TYPE msg_type);
	};
}

#endif
