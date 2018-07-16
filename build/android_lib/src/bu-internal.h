#include <utils/headers.h>
#include <common/general.h>
#include <common/storage.h>
#include <common/private_key.h>
#include <common/argument.h>
#include <common/daemon.h>
#include <overlay/peer_manager.h>
#include <ledger/ledger_manager.h>
#include <consensus/consensus_manager.h>
#include <glue/glue_manager.h>
#include <api/web_server.h>
#include <api/websocket_server.h>
#include <api/console.h>
#include <ledger/contract_manager.h>
#include <monitor/monitor_manager.h>

#include <time.h>

void SaveWSPort();
void RunLoop();

class BuMaster : public utils::Singleton<BuMaster>, public utils::Runnable 
{
public:
	BuMaster();
	~BuMaster();

	bool Initialize(const std::string &bu_home_path);
	bool Exit();
private:
	utils::Thread *thread_ptr_;
	virtual void Run(utils::Thread *thread);
	int MainLoop(int argc, char *argv[]);

	std::string bu_home_path_;
};
namespace utils {
	void android_log(const char* file_name, const char * format, ...);
}


