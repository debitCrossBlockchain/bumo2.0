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

#ifndef PROPOSER_MANAGER_H_
#define PROPOSER_MANAGER_H_

#include <utils/singleton.h>
#include <utils/thread.h>
#include<cross/message_channel_manager.h>

namespace bumo {
	class ProposerManager :public utils::Singleton<ProposerManager>,
		public bumo::IMessageChannelConsumer,
		public utils::Runnable{
		friend class utils::Singleton<bumo::ProposerManager>;
	public:
		ProposerManager();
		~ProposerManager();

		bool Initialize();
		bool Exit();
		bool CommitChildChainBlock();
		void HandleMessageChannelConsumer(const protocol::MessageChannel &message_channel);

	private:
		virtual void Run(utils::Thread *thread) override;
		void HandleChildChainBlock();
		bool CheckChildBlockExsit();
		bool CommitTransaction();
		bool AddressIsValidate(const std::string &address);
		void UpdateValidateAddressList();

		bool enabled_;
		utils::Thread *thread_ptr_;
		utils::Mutex validate_address_lock_;
		utils::StringList validate_address_;
		int64_t last_uptate_validate_address_time_;
	};

}

#endif
