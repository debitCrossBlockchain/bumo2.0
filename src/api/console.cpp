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

#include <json/json.h>
#include <utils/headers.h>
#include <common/key_store.h>
#include <ledger/ledger_manager.h>
#include "console.h"

namespace bumo {
	Console::Console() {
		thread_ptr_ = NULL;
		priv_key_ = NULL;
		funcs_["createWallet"] = std::bind(&Console::CreateWallet, this, std::placeholders::_1);
		funcs_["openWallet"] = std::bind(&Console::OpenWallet, this, std::placeholders::_1);
		funcs_["closeWallet"] = std::bind(&Console::CloseWallet, this, std::placeholders::_1);
		funcs_["getBalance"] = std::bind(&Console::GetBalance, this, std::placeholders::_1);
		funcs_["getBlockNumber"] = std::bind(&Console::GetBlockNumber, this, std::placeholders::_1);
		funcs_["help"] = std::bind(&Console::Usage, this, std::placeholders::_1);
		funcs_["getAddress"] = std::bind(&Console::GetAddress, this, std::placeholders::_1);
	}

	Console::~Console() {
		if (thread_ptr_) {
			delete thread_ptr_;
		}

		if (priv_key_){
			delete priv_key_;
		}
	}

	bool Console::Initialize() {
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start("console")) {
			return false;
		}
		return true;
	}

	bool Console::Exit() {
		return true;
	}

	extern bool g_enable_;
	void Console::Run(utils::Thread *thread) {
		while (g_enable_) {
			std::string input;
			std::cout << "> ";
			std::getline(std::cin, input);
			utils::StringVector args = utils::String::Strtok(input, ' ');
			if (args.size() < 1) continue;

			ConsolePocMap::iterator iter = funcs_.find(args[0]);
			if (iter != funcs_.end()){
				iter->second(args);
			}
		}
	}

	void Console::OpenWallet(const utils::StringVector &args) {
		std::string password;
		if (args.size() > 1) {

			if (!utils::File::IsExist(args[1])) {
				std::cout << "path (" << args[1] << ") not exist" << std::endl;
				return;
			}

			password = utils::GetCinPassword("input the password:");
			std::cout << std::endl;
			if (password.empty()) {
				std::cout << "error, empty" << std::endl;
				return;
			}
		}
		else {
			return;
		}

		utils::File file_object;
		if (!file_object.Open(args[1], utils::File::FILE_M_READ)) {
			std::string error_info = utils::String::Format("open failed, error desc(%s)", STD_ERR_DESC);
			std::cout << error_info << std::endl;
			return;
		}
		std::string serial_str;
		file_object.ReadData(serial_str, 1 * utils::BYTES_PER_MEGA);

		Json::Value key_store_json;
		if (!key_store_json.fromString(serial_str)){
			std::cout << "parse string failed" << std::endl;
			return;
		}

		KeyStore key_store;
		std::string restore_priv_key;
		bool ret = key_store.From(key_store_json, password, restore_priv_key);
		if (ret) {
			if (priv_key_) {
				delete priv_key_;
				priv_key_ = NULL;
			}
			priv_key_ = new PrivateKey(restore_priv_key);
			std::cout << "ok" << std::endl;
		}
		else {
			std::cout << "error" <<std::endl;
		}
	}

	void Console::CreateWallet(const utils::StringVector &args) {
		std::string password;
		if (args.size() > 1) {

			if (utils::File::IsExist(args[1])) {
				std::cout << "path (" << args[1] << ") exist" << std::endl;
				return;
			}

			password = utils::GetCinPassword("input the password:");
			std::cout << std::endl;
			if (password.empty()) {
				std::cout << "error, empty" << std::endl;
				return;
			}
			std::string password1 = utils::GetCinPassword("input the password verified:");
			std::cout << std::endl;
			if (password != password1) {
				std::cout << "error, not match" << std::endl;
				return;
			}
		}
		else {
			return;
		}

		utils::File file_object;
		if (!file_object.Open(args[1], utils::File::FILE_M_WRITE)) {
			std::string error_info = utils::String::Format("create failed, error desc(%s)", STD_ERR_DESC);
			std::cout << error_info << std::endl;
			return;
		}

		KeyStore key_store;
		Json::Value key_store_json;
		std::string new_priv_key;
		bool ret = key_store.Generate(password, key_store_json, new_priv_key);
		if (ret) {
			std::string serial_str = key_store_json.toFastString();
			std::cout << serial_str << std::endl;
			file_object.Write(serial_str.c_str(), 1, serial_str.size());
			if (priv_key_) {
				delete priv_key_;
				priv_key_ = NULL;
			}
			priv_key_ = new PrivateKey(new_priv_key);
		}
		else {
			std::cout << "error" << std::endl;
		}
	}

	void Console::CloseWallet(const utils::StringVector &args) {
		if (priv_key_ != NULL) {
			delete priv_key_;
			priv_key_ = NULL;
		} 
		std::cout  << "ok" << std::endl;
	}

	void Console::GetBlockNumber(const utils::StringVector &args) {
		std::cout << LedgerManager::Instance().GetLastClosedLedger().seq() << std::endl;
	}

	void Console::GetBalance(const utils::StringVector &args) {
		std::string address;
		if (args.size() < 2){
			if (priv_key_ != NULL) {
				address = priv_key_->GetEncAddress();
			} 
			else {
				std::cout << "error, please input the address" << std::endl;
				return;
			}
		} 
		else {
			address = args[1];
		}

		AccountFrm::pointer account_ptr;
		if (!Environment::AccountFromDB(address, account_ptr)) {
			std::cout << "error " << address << " not exist" << std::endl;
		}
		else {
			std::cout << account_ptr->GetAccountBalance() << std::endl;
		}
	}

	void Console::GetAddress(const utils::StringVector &args) {
		if (priv_key_ != NULL) {
			std::cout << priv_key_->GetEncAddress() << std::endl;
		} 
		else {
			std::cout << "error, wallet not opened" << std::endl;
		}
	}

	void Console::Usage(const utils::StringVector &args) {
		printf(
			"Usage: bumo [OPTIONS]\n"
			"OPTIONS:\n"
			"createWallet <path>                        create wallet\n"
			"openWallet <path>                          open keystore\n"
			"payCoin <to-address> <bu coin> <fee>          \n"
			"getBalance <account>                          \n"
			"getBlockNumber <account>                          \n"
			"closeWallet \n"
			);
	}
}
