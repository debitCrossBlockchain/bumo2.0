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
#include <cross/cross_utils.h>
#include<cross/message_channel_manager.h>
#include <cross/message_handler.h>
#include<cross/cross_manager.h>
#include "configure.h"

void SaveWSPort();
void RunLoop();
int main(int argc, char *argv[]){

#ifdef WIN32
	_set_output_format(_TWO_DIGIT_EXPONENT);
#else
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	size_t stacksize = 0;
	int ret = pthread_attr_getstacksize(&attr, &stacksize);
	if (ret != 0) {
		printf("get stacksize error!:%d\n", (int)stacksize);
		return -1;
	}

	if (stacksize <= 2 * 1024 * 1024)
	{
		stacksize = 2 * 1024 * 1024;

		pthread_attr_t object_attr;
		pthread_attr_init(&object_attr);
		ret = pthread_attr_setstacksize(&object_attr, stacksize);
		if (ret != 0) {
			printf("set main stacksize error!:%d\n", (int)stacksize);
			return -1;
		}
	}
#endif

	utils::SetExceptionHandle();
	utils::Thread::SetCurrentThreadName("bumo-thread");

	utils::Daemon::InitInstance();
	utils::net::Initialize();
	utils::Timer::InitInstance();
	bumo::Configure::InitInstance();
	bumo::Storage::InitInstance();
	bumo::Global::InitInstance();
	bumo::SlowTimer::InitInstance();
	utils::Logger::InitInstance();
	bumo::TransactionSender::InitInstance();
	bumo::MessageChannel::InitInstance();
	bumo::MessageHandler::InitInstance();
	bumo::Console::InitInstance();
	bumo::PeerManager::InitInstance();
	bumo::LedgerManager::InitInstance();
	bumo::ConsensusManager::InitInstance();
	bumo::GlueManager::InitInstance();
	bumo::WebSocketServer::InitInstance();
	bumo::WebServer::InitInstance();
	bumo::MonitorManager::InitInstance();
	bumo::ContractManager::InitInstance();
	bumo::CrossManager::InitInstance();

	bumo::Argument arg;
	if (arg.Parse(argc, argv)){
		return 1;
	}

	do {
		utils::ObjectExit object_exit;
		bumo::InstallSignal();

		if (arg.console_){
			arg.log_dest_ = utils::LOG_DEST_FILE; //Cancel the std output
			bumo::Console &console = bumo::Console::Instance();
			console.Initialize();
			object_exit.Push(std::bind(&bumo::Console::Exit, &console));
		}

		srand((uint32_t)time(NULL));
		bumo::StatusModule::modules_status_ = new Json::Value;
#ifndef OS_MAC
		utils::Daemon &daemon = utils::Daemon::Instance();
		if (!bumo::g_enable_ || !daemon.Initialize((int32_t)1234))
		{
			LOG_STD_ERRNO("Failed to initialize daemon", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		object_exit.Push(std::bind(&utils::Daemon::Exit, &daemon));
#endif

		bumo::Configure &config = bumo::Configure::Instance();
		std::string config_path = bumo::General::CONFIG_FILE;
		if (!utils::File::IsAbsolute(config_path)){
			config_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), config_path.c_str());
		}

		if (!config.Load(config_path)){
			LOG_STD_ERRNO("Failed to load configuration", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}

		std::string log_path = config.logger_configure_.path_;
		if (!utils::File::IsAbsolute(log_path)){
			log_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), log_path.c_str());
		}
		const bumo::LoggerConfigure &logger_config = bumo::Configure::Instance().logger_configure_;
		utils::Logger &logger = utils::Logger::Instance();
		logger.SetCapacity(logger_config.time_capacity_, logger_config.size_capacity_);
		logger.SetExpireDays(logger_config.expire_days_);
		if (!bumo::g_enable_ || !logger.Initialize((utils::LogDest)(arg.log_dest_ >= 0 ? arg.log_dest_ : logger_config.dest_),
			(utils::LogLevel)logger_config.level_, log_path, true)){
			LOG_STD_ERR("Failed to initialize logger");
			break;
		}
		object_exit.Push(std::bind(&utils::Logger::Exit, &logger));
		LOG_INFO("Initialized daemon successfully");
		LOG_INFO("Loaded configure successfully");
		LOG_INFO("Initialized logger successfully");

		// end run command
		bumo::Storage &storage = bumo::Storage::Instance();
		LOG_INFO("The path of the database is as follows: keyvalue(%s),account(%s),ledger(%s)", 
			config.db_configure_.keyvalue_db_path_.c_str(),
			config.db_configure_.account_db_path_.c_str(),
			config.db_configure_.ledger_db_path_.c_str());

		if (!bumo::g_enable_ || !storage.Initialize(config.db_configure_, arg.drop_db_)) {
			LOG_ERROR("Failed to initialize database");
			break;
		}
		object_exit.Push(std::bind(&bumo::Storage::Exit, &storage));
		LOG_INFO("Initialized database successfully");

		if (arg.drop_db_) {
			LOG_INFO("Droped database successfully");
			return 1;
		} 
		
		if ( arg.clear_consensus_status_ ){
			bumo::Pbft::ClearStatus();
			LOG_INFO("Cleared consensus status successfully");
			return 1;
		}

		if (arg.clear_peer_addresses_) {
			bumo::KeyValueDb *db = bumo::Storage::Instance().keyvalue_db();
			db->Put(bumo::General::PEERS_TABLE, "");
			LOG_INFO("Cleared peer addresss list successfully");
			return 1;
		} 

		if (arg.create_hardfork_) {
			bumo::LedgerManager &ledgermanger = bumo::LedgerManager::Instance();
			if (!ledgermanger.Initialize()) {
				LOG_ERROR("Failed to initialize legder manger!");
				return -1;
			}
			bumo::LedgerManager::CreateHardforkLedger();
			return 1;
		}

		bumo::Global &global = bumo::Global::Instance();
		if (!bumo::g_enable_ || !global.Initialize()){
			LOG_ERROR_ERRNO("Failed to initialize global variable", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		object_exit.Push(std::bind(&bumo::Global::Exit, &global));
		LOG_INFO("Initialized global module successfully");


		bumo::TransactionSender &trans_sender = bumo::TransactionSender::Instance();
		if (!trans_sender.Initialize(bumo::Configure::Instance().ledger_configure_.validation_privatekey_)){
			LOG_ERROR("Failed to trans sender");
			break;
		}
		object_exit.Push(std::bind(&bumo::TransactionSender::Exit, &trans_sender));
		LOG_INFO("Initialized trans sender successfully");

		bumo::MessageChannel &message_channel = bumo::MessageChannel::Instance();
		if (!message_channel.Initialize(bumo::Configure::Instance().message_channel_configure_)){
			LOG_ERROR_ERRNO("Failed to initialize message channel", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		object_exit.Push(std::bind(&bumo::MessageChannel::Exit, &message_channel));
		LOG_INFO("Initialized message channel successfully");

		bumo::MessageHandler &message_handler = bumo::MessageHandler::Instance();
		if (!bumo::g_enable_ || !message_handler.Initialize()){
			LOG_ERROR_ERRNO("Failed to initialize message handler", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		object_exit.Push(std::bind(&bumo::MessageHandler::Exit, &message_handler));
		LOG_INFO("Initialized message handler successfully");


		//Consensus manager must be initialized before ledger manager and glue manager
		bumo::ConsensusManager &consensus_manager = bumo::ConsensusManager::Instance();
		if (!bumo::g_enable_ || !consensus_manager.Initialize(bumo::Configure::Instance().ledger_configure_.validation_type_)) {
			LOG_ERROR("Failed to initialize consensus manager");
			break;
		}
		object_exit.Push(std::bind(&bumo::ConsensusManager::Exit, &consensus_manager));
		LOG_INFO("Initialized consensus manager successfully");

		bumo::LedgerManager &ledgermanger = bumo::LedgerManager::Instance();
		if (!bumo::g_enable_ || !ledgermanger.Initialize()) {
			LOG_ERROR("Failed to initialize ledger manager");
			break;
		}
		object_exit.Push(std::bind(&bumo::LedgerManager::Exit, &ledgermanger));
		LOG_INFO("Initialized ledger successfully");

		bumo::GlueManager &glue = bumo::GlueManager::Instance();
		if (!bumo::g_enable_ || !glue.Initialize()){
			LOG_ERROR("Failed to initialize glue manager");
			break;
		}
		object_exit.Push(std::bind(&bumo::GlueManager::Exit, &glue));
		LOG_INFO("Initialized glue manager successfully");

		bumo::PeerManager &p2p = bumo::PeerManager::Instance();
		if (!bumo::g_enable_ || !p2p.Initialize(NULL, false)) {
			LOG_ERROR("Failed to initialize peer network");
			break;
		}
		object_exit.Push(std::bind(&bumo::PeerManager::Exit, &p2p));
		LOG_INFO("Initialized peer network successfully");

		bumo::SlowTimer &slow_timer = bumo::SlowTimer::Instance();
		if (!bumo::g_enable_ || !slow_timer.Initialize(1)){
			LOG_ERROR_ERRNO("Failed to initialize slow timer", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		object_exit.Push(std::bind(&bumo::SlowTimer::Exit, &slow_timer));
		LOG_INFO("Initialized slow timer with " FMT_SIZE " successfully", utils::System::GetCpuCoreCount());

		bumo::WebSocketServer &ws_server = bumo::WebSocketServer::Instance();
		if (!bumo::g_enable_ || !ws_server.Initialize(bumo::Configure::Instance().wsserver_configure_)) {
			LOG_ERROR("Failed to initialize web server");
			break;
		}
		object_exit.Push(std::bind(&bumo::WebSocketServer::Exit, &ws_server));
		LOG_INFO("Initialized web server successfully");

		bumo::WebServer &web_server = bumo::WebServer::Instance();
		if (!bumo::g_enable_ || !web_server.Initialize(bumo::Configure::Instance().webserver_configure_)) {
			LOG_ERROR("Failed to initialize web server");
			break;
		}
		object_exit.Push(std::bind(&bumo::WebServer::Exit, &web_server));
		LOG_INFO("Initialized web server successfully");

		SaveWSPort();
		
		bumo::MonitorManager &monitor_manager = bumo::MonitorManager::Instance();
		if (!bumo::g_enable_ || !monitor_manager.Initialize()) {
			LOG_ERROR("Failed to initialize monitor manager");
			break;
		}
		object_exit.Push(std::bind(&bumo::MonitorManager::Exit, &monitor_manager));
		LOG_INFO("Initialized monitor manager successfully");

		bumo::ContractManager &contract_manager = bumo::ContractManager::Instance();
		if (!contract_manager.Initialize(argc, argv)){
			LOG_ERROR("Failed to initialize contract manager");
			break;
		}
		object_exit.Push(std::bind(&bumo::ContractManager::Exit, &contract_manager));
		LOG_INFO("Initialized contract manager successfully");

		bumo::CrossManager &cross_manager = bumo::CrossManager::Instance();
		if (!cross_manager.Initialize()){
			LOG_ERROR("Failed to initialize cross manager");
			break;
		}
		object_exit.Push(std::bind(&bumo::CrossManager::Exit, &cross_manager));
		LOG_INFO("Initialized cross manager successfully");

		bumo::g_ready_ = true;

		RunLoop();

		LOG_INFO("Process begins to quit...");
		delete bumo::StatusModule::modules_status_;

	} while (false);

	bumo::ContractManager::ExitInstance();
	bumo::SlowTimer::ExitInstance();
	bumo::GlueManager::ExitInstance();
	bumo::LedgerManager::ExitInstance();
	bumo::PeerManager::ExitInstance();
	bumo::WebSocketServer::ExitInstance();
	bumo::WebServer::ExitInstance();
	bumo::MonitorManager::ExitInstance();
	bumo::MessageHandler::ExitInstance();
	bumo::TransactionSender::ExitInstance();
	bumo::Configure::ExitInstance();
	bumo::Global::ExitInstance();
	bumo::Storage::ExitInstance();
	utils::Logger::ExitInstance();
	utils::Daemon::ExitInstance();
	bumo::CrossManager::ExitInstance();
	bumo::MessageChannel::ExitInstance();
	
	if (arg.console_ && !bumo::g_ready_) {
		printf("Initialized failed, please check log for detail\n");
	}
	printf("process exit\n");
}

void RunLoop(){
	int64_t check_module_interval = 5 * utils::MICRO_UNITS_PER_SEC;
	int64_t last_check_module = 0;
	while (bumo::g_enable_){
		int64_t current_time = utils::Timestamp::HighResolution();

		for (auto item : bumo::TimerNotify::notifys_){
			item->TimerWrapper(utils::Timestamp::HighResolution());
			if (item->IsExpire(utils::MICRO_UNITS_PER_SEC)){
				LOG_WARN("The execution time(" FMT_I64 " us) for the timer(%s) is expired after 1s elapses", item->GetLastExecuteTime(), item->GetTimerName().c_str());
			}
		}

		utils::Timer::Instance().OnTimer(current_time);
		utils::Logger::Instance().CheckExpiredLog();

		if (current_time - last_check_module > check_module_interval){
			utils::WriteLockGuard guard(bumo::StatusModule::status_lock_);
			bumo::StatusModule::GetModulesStatus(*bumo::StatusModule::modules_status_);
			last_check_module = current_time;
		}

		utils::Sleep(1);
	}
}

void SaveWSPort(){    
    std::string tmp_file = utils::File::GetTempDirectory() +"/bumo_listen_port";
	Json::Value json_port = Json::Value(Json::objectValue);
	json_port["webserver_port"] = bumo::WebServer::Instance().GetListenPort();
	json_port["wsserver_port"] = bumo::WebSocketServer::Instance().GetListenPort();
	utils::File file;
	if (file.Open(tmp_file, utils::File::FILE_M_WRITE | utils::File::FILE_M_TEXT))
	{
		std::string line = json_port.toFastString();
		file.Write(line.c_str(), 1, line.length());
		file.Close();
	}
}
