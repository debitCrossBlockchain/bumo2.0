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

#include "general.h"
#include "private_key.h"
#include "argument.h"

namespace bumo {
	bool g_enable_ = true;
	Argument::Argument() :
		help_modle_(false),
		drop_db_(false),
		peer_addr_(false),
		clear_peer_addresses_(false),
		clear_consensus_status_(false),
		create_hardfork_(false){}
	Argument::~Argument() {}

	bool Argument::Parse(int argc, char *argv[]) {
		if (argc > 1) {
			std::string s(argv[1]);
			if (s == "--dropdb") {
				drop_db_ = true;
			}
			else if (s == "--hardware-address") {
				ShowHardwareAddress();
				return true;
			}
			else if (s == "--clear-peer-addresses") {
				clear_peer_addresses_ = true;
			}
			else if (s == "--aes-crypto") {
				if (argc > 2) {
					printf("%s\n", (utils::Aes::CryptoHex(argv[2], bumo::GetDataSecuretKey())).c_str());
				}
				else {
					printf("missing parameter, need crypto value \n");
				}
				return true;
			}
			//else if (s == "--aes-decrypt"  && argc > 2) {
			//	printf("%s\n", utils::Aes::HexDecrypto(argv[2], bubi::GetDataSecuretKey()).c_str());
			//	return true;
			//}
			else if (s == "--sm3" ) {
				if (argc > 2) {
					printf("%s\n", utils::String::BinToHexString(utils::Sm3::Crypto(argv[2])).c_str());
				}
				else {
					printf("missing parameter, need crypto value \n");
				}
				return true;
			}
			else if (s == "--sm3-hex" ) {
				if (argc > 2) {
					printf("%s\n", utils::String::BinToHexString(utils::Sm3::Crypto(utils::String::HexStringToBin(argv[2]))).c_str());
				}
				else {
					printf("missing parameter, need crypto value with hex format \n");
				}
				return true;
			}
			else if (s == "--peer-address") {
				ShowNodeId(argc, argv);
				return true;
			}
			else if (s == "--clear-consensus-status") {
				clear_consensus_status_ = true;
			}
			else if (s == "--create-hardfork") {
				create_hardfork_ = true;
			}
			else if (s == "--version") {
#ifdef SVNVERSION
				printf("%s,%u; " SVNVERSION "\n", General::BUMO_VERSION, General::LEDGER_VERSION);
#else
				printf("%s,%u\n", General::BUMO_VERSION, General::LEDGER_VERSION);
#endif 
				return true;
			}
			else if (s == "--help") {
				Usage();
				return true;
			}
			else if (s == "--check-signed-data" && argc > 4) {
				printf("%s\n", PublicKey::Verify(utils::String::HexStringToBin(argv[2]), utils::String::HexStringToBin(argv[3]), argv[4]) ? "true" : "false");
				return true;
			}
			else if (s == "--sign-data" && argc > 3) {
				PrivateKey priv_key(argv[2]);
				std::string public_key = priv_key.GetEncPublicKey();
				std::string raw_data = utils::String::HexStringToBin(argv[3]);
				Json::Value result = Json::Value(Json::objectValue);
				
				result["data"] = argv[3];
				result["public_key"] = public_key;
				result["sign_data"] = utils::String::BinToHexString(priv_key.Sign(raw_data));
				printf("%s\n", result.toStyledString().c_str());
				return true;
			}
			else if (s == "--get-address" && argc > 2) {
				PrivateKey priv_key(argv[2]);
				std::string public_key = priv_key.GetEncPublicKey();
				std::string private_key = priv_key.GetEncPrivateKey();
				std::string public_address = priv_key.GetEncAddress();
				Json::Value result = Json::Value(Json::objectValue);

				result["public_key"] = public_key;
				result["private_key"] = private_key;
				result["private_key_aes"] = utils::Aes::CryptoHex(private_key, bumo::GetDataSecuretKey());
				result["address"] = public_address;
				result["public_key_raw"] = EncodePublicKey(priv_key.GetRawPublicKey());
				result["sign_type"] = GetSignTypeDesc(priv_key.GetSignType());
				printf("%s\n", result.toStyledString().c_str());
				return true;
			}
			else if (s == "--create-account" && argc > 2) {
				SignatureType type = GetSignTypeByDesc(argv[2]);
				if (type == SIGNTYPE_NONE) {
					printf("parameter \"%s\" error, support ed25519 \n", argv[2]);
					return true;
				} 

				PrivateKey priv_key(type);
                std::string public_key = priv_key.GetEncPublicKey();
                std::string private_key = priv_key.GetEncPrivateKey();
                std::string public_address = priv_key.GetEncAddress();

				LOG_TRACE("Creating account address:%s", public_address.c_str());
				Json::Value result = Json::Value(Json::objectValue);
				result["public_key"] = public_key;
				result["private_key"] = private_key;
				result["private_key_aes"] = utils::Aes::CryptoHex(private_key, bumo::GetDataSecuretKey());
				result["address"] = public_address;
				result["public_key_raw"] = EncodePublicKey(priv_key.GetRawPublicKey());
				result["sign_type"] = GetSignTypeDesc(priv_key.GetSignType());
				printf("%s\n", result.toStyledString().c_str());
				return true;
			}
			else if (s == "--dbtool") {
				printf("input database path:\n");
				std::string path;
				std::cin >> path;
				KeyValueDb* ledger_db_ = nullptr;
#ifdef WIN32
				ledger_db_ = new LevelDbDriver();
#else
				ledger_db_ = new RocksDbDriver();
#endif
				if (!ledger_db_->Open(path)) {
					return false;
				}

				printf("1:list all key and values\n");
				printf("2:query one key\n");
				char ch;
				std::cin >> ch;
				if (ch == '1'){
#ifdef WIN32
					auto it = (leveldb::Iterator*)ledger_db_->NewIterator();
#else
					auto it = (rocksdb::Iterator*)ledger_db_->NewIterator();
#endif
					for (it->SeekToFirst(); it->Valid(); it->Next()){
						printf("%s:%s\n", utils::String::BinToHexString(it->key().ToString()).c_str(),
							utils::String::BinToHexString(it->value().ToString()).c_str());
					}
				}
				else if (ch == '2')
					while (true){
						printf("\ninput key(hex):");
						std::string hexkey, buff;
						std::cin >> hexkey;
						auto binkey = utils::String::HexStringToBin(hexkey);
						if (ledger_db_->Get(binkey, buff)){
							printf("%s", utils::String::BinToHexString(buff).c_str());
						}
						else{
							printf("%s", ledger_db_->error_desc().c_str());
						}
					}
				return true;
			}
			else {
				Usage();
				return true;
			}
		}

		return false;
	}

	void Argument::Usage() {
		printf(
			"Usage: bumo [OPTIONS]\n"
			"OPTIONS:\n"
			"  --dropdb                        clean up database\n"
			"  --peer-address <node-priv-key>  get peer address from crypted node private key\n"
			"  --create-account <crypto>       create account, support ed25519\n"
			"  --get-address <node-priv-key>   get address from private key"
			"  --sign-data <node-priv-key> <blob data>   sign blob data"
			"  --check-signed-data <blob data> <signed data> <public key> check signed data"
			"  --hardware-address              get local hardware address\n"
			"  --clear-consensus-status        delete consensus status\n"
			"  --sm3 <arg>                     generate sm3 hash \n"
			"  --sm3-hex <arg>                 generate sm3 hash from hex format \n"
			"  --aes-crypto <value>            crypto value\n"
			"  --version                       display version information\n"
			"  --create-hardfork               create hard fork ledger\n"
			"  --clear-peer-addresses          clear peer list\n"
			"  --help                          display this help\n"
			);
	}

	void Argument::ShowNodeId(int argc, char *argv[]) {
		if (argc < 3) {
			printf("missing parameter, need 1 parameter (the aes_crypto code of private key)\n");
			return;
		}

		if (!utils::String::IsHexString(argv[2])) {
			printf("the node_id of inputting is invalid, please check it!\n");
			return;
		}

		bumo::PrivateKey private_key(utils::Aes::HexDecrypto(argv[2], bumo::GetDataSecuretKey()));

        printf("local peer address (%s)\n", private_key.GetEncAddress().c_str());
	}

	void Argument::ShowHardwareAddress() {
		std::string hard_address = "";
		utils::System system;
		char out_msg[256] = { 0 };
		if (system.GetHardwareAddress(hard_address, out_msg))
			printf("local hardware address (%s)\n", hard_address.c_str());
		else
			printf("%s\n", out_msg);
	}

	void SignalFunc(int32_t code) {
		fprintf(stderr, "Get quit signal(%d)\n", code);
		g_enable_ = false;
	}

	void InstallSignal() {
		signal(SIGHUP, SignalFunc);
		signal(SIGQUIT, SignalFunc);
		signal(SIGINT, SignalFunc);
		signal(SIGTERM, SignalFunc);
#ifndef WIN32
		signal(SIGPIPE, SIG_IGN);
#endif
	}

}