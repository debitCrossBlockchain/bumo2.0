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

namespace bumo {
	class BlockListenManager :public utils::Singleton<BlockListenManager>{
		friend class utils::Singleton<bumo::BlockListenManager>;
	public:
		BlockListenManager();
		~BlockListenManager();

		bool Initialize();
		bool Exit();
		bool CommitBlock();

	private:
		void HandleMainChainBlock();
		void HandleChildChainBlock();
	};
}

#endif
