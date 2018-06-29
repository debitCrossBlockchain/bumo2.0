#ifndef TIDB_H_
#define TIDB_H_

#ifdef WIN32
#include <winsock2.h>
#include "mysql/mysql.h"
#else
#include "mysql.h"
#endif
#include <string>
#include <map>
#include <utils/headers.h>

using namespace std;

#define TIDB_KV_DB		"KVDB"
#define TIDB_LEDGER_DB	"LEDGERDB"
#define TIDB_ACCOUNT_DB	"ACCOUNTDB"

#define STMT_NUM	20

#define REPLACE_BATCH_NUM	10

namespace bumo{

	typedef void(*Call_back)(MYSQL_ROW, int, unsigned long *,void *);

	class SqlDb {

	protected:
		std::string error_desc_;
	public:
		SqlDb() {}
		~SqlDb() {}

		virtual int connect(const char *host, const char *username, const char *password,
			const char *database, int port, const char * unixsocket, int flag) = 0;

		virtual int close() = 0;

		virtual int exec_sql(const char *sql, Call_back  call_back = NULL, void *param = NULL) = 0;

		virtual void* NewIterator() = 0;
	};

	class MysqlDriver   {
	private:
		MYSQL * m_pMysql;
		//0~9 = replace()¡£10~14=select
		MYSQL_STMT *stmt[STMT_NUM];
		
	public:
		//
		MysqlDriver();
		~MysqlDriver();

		
		//desc£ºconnect mysql
		//input£º	host, username,password, database, port, unixsocket,  flag
		//output£º	
		//return£º	error		Nonzero 
		//			success		Zero 
		int mysql_connect(const char *host, const char *username, const char *password, 
			const char *database, int port, const char * unixsocket, int flag);

		//desc£ºclose mysql connect
		void close_mysql();

		
		//desc£ºexcute sql statement
		//input£º	sql				sql statement
		//			Call_back		if sql is select statement,need Call_back deal with the result set
		//							or not select please input null or do nothing;
		//output£º	param			save the dealed data 
		//return£º	select/show...			result set rows
		//			insert/delete...		affected rows
		int64_t mysql_exec(const char *sql, Call_back  call_back = NULL, void *param = NULL);

		//desc£ºget the last mysql error msg
		std::string get_err_str();

		//desc£ºswitch database
		//input£ºdb_name			database name
		//output£º
		//return£ºZero for success. Nonzero if an error occurred.
		int64_t select_db(const char*db_name);

		//desc£º The special characters in the SQL statement need be escaped 
		//input£ºorgStr			string need be escaped
		//		 iLen			orgStr length
		//output£ºsv_str		saved string for escaped str, at leaset (2*iLen+1) space have been allocate
		//return£ºescaped string length.
		long format_blob_string(char *sv_str, const char *orgStr, int iLen);

		int32_t do_commit();

		int32_t roll_back();


		bool init_stmt(const char* stmt_sql, int iPos);

		int64_t stmt_exec(MYSQL_BIND *param, int stmt_series);


	private:
		int64_t do_sql(const char *sql, Call_back  call_back = NULL, void *param = NULL);

		int64_t mysql_select(const char *sql, Call_back  call_back, void *param);

		

	};

	class WriteTidbBatch
	{
	public:
		WriteTidbBatch() {}
		~WriteTidbBatch(){}

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
	
	class tidb /*: public KeyValueDb*/ {



	public:
		tidb(const std::string host_ip, const std::string user_name, const std::string pwd,int32_t port );
		~tidb();

		//db_name  database name
		bool Open(const std::string &db_name);
		bool Close();
		int64_t Get(const std::string &key, std::string &value);
		bool Put(const std::string &key, const std::string &value) ;
		bool Delete(const std::string &key);
		//bool GetOptions(Json::Value &options) ;
		std::string get_error() {
			return m_pMysqlDriver->get_err_str();
		}

		bool Put(WriteTidbBatch &value);

		int64_t Get_All_Ledger(std::map<string, string> &_out_map);

		
	private:
		//create database
		bool initDB(const char *db_name);

		//create kv_table£¬the first table for example
		bool create_kv_table();

		//init config
		bool initialize();

		//create database
		bool init_stmt();

		int64_t do_replace_stmt(std::map<std::string, std::string> &in_map);

		int64_t do_replace_stmt(const std::string &key, const std::string &value);
		

	private:
		//mysql driver
		MysqlDriver *m_pMysqlDriver;

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