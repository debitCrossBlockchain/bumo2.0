#ifndef COCKROACHDB_H_
#define COCKROACHDB_H_

#ifdef WIN32
#include "pqxx/pqxx"
#else
#include <pqxx/pqxx>
#endif
#include <string>
#include <map>
#include <utils/headers.h>

using namespace std;

#define COCKROACHDB_KV_DB		"KVDB"
#define COCKROACHDB_LEDGER_DB	"LEDGERDB"
#define COCKROACHDB_ACCOUNT_DB	"ACCOUNTDB"

#define STMT_NUM	20

#define REPLACE_BATCH_NUM	10

namespace bumo{

	
	class WriteCodbBatch
	{
	public:
		WriteCodbBatch() {}
		~WriteCodbBatch(){}

		// Store the mapping "key->value" in the database.
		void Put(const std::string& key, const std::string& value)
		{
			put_op_map[key] = value;
		}


		// If the database contains a mapping for "key", erase it.  Else do nothing.
		void Delete(const std::string& key)
		{
			put_op_map.erase(key);
		}

		// Clear all updates buffered in this batch.
		void Clear()
		{
			put_op_map.clear();
		}

		std::map<std::string, std::string> &get_put_map()
		{
			return put_op_map;
		}

	private:
		std::map<std::string, std::string> put_op_map;
	};
	
	class codb /*: public KeyValueDb*/ {



	public:
		codb(const std::string host_ip, const std::string user_name, const std::string pwd, int32_t port);
		~codb();

		//db_name  database name
		bool Open(const std::string &db_name);
		bool Close();
		int64_t Get(const std::string &key, std::string &value);
		bool Put(const std::string &key, const std::string &value) ;
		bool Delete(const std::string &key);
		//bool GetOptions(Json::Value &options) ;
		std::string get_error() {
			return "";
		}

		bool Put(WriteCodbBatch &value);

		int64_t Get_All_Ledger(std::map<string, string> &_out_map);

		
	private:

		//create kv_table£¬the first table for example
		bool create_kv_table();

		//init config
		bool initialize();

		//create database
		bool init_stmt();

				

	private:
		//mysql driver

		pqxx::connection *c1;
		pqxx::connection *c2;

		//tidb host
		std::string m_Host_ip ;
		//tidb user
		std::string m_User_name;
		//tidb pass
		std::string m_Password;
		//tidb port
		int32_t m_Port;

	};


}
#endif