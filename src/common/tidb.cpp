
#include "tidb.h"
#include <string.h>
#include <iostream>
#include <utils/strings.h>
#include <utils/logger.h>
#include <sstream>
#include <chrono>

//time define for time_diff ,ms unit
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;


using namespace std;


namespace bumo {



	const char *IsNull(char *source)
	{
		if (NULL == source)
			return "";
		return source;
	}

	void get_table_value(MYSQL_ROW row, int n_column, unsigned long * len, void * param)
	{
		if (row)
		{
			string * strValue = (string*)param;
			strValue->append(IsNull(row[1]), len[1]);
		}
	}

	void get_count(MYSQL_ROW row, int n_column, unsigned long * len, void * param)
	{
		int* iCount = (int*)param;
		if (row)
		{
			*iCount = atoi(row[0]);
		};
	}

	//just for dbg hex
	void HexDump(const char *buf, int len, int addr) {
		int i, j, k;
		char binstr[80];

		for (i = 0; i < len; i++) {
			if (0 == (i % 16)) {
				sprintf(binstr, "%08x -", i + addr);
				sprintf(binstr, "%s %02x", binstr, (unsigned char)buf[i]);
			}
			else if (15 == (i % 16)) {
				sprintf(binstr, "%s %02x", binstr, (unsigned char)buf[i]);
				sprintf(binstr, "%s  ", binstr);
				for (j = i - 15; j <= i; j++) {
					sprintf(binstr, "%s%c", binstr, ('!' < buf[j] && buf[j] <= '~') ? buf[j] : '.');
				}
				printf("%s\n", binstr);
			}
			else {
				sprintf(binstr, "%s %02x", binstr, (unsigned char)buf[i]);
			}
		}
		if (0 != (i % 16)) {
			k = 16 - (i % 16);
			for (j = 0; j < k; j++) {
				sprintf(binstr, "%s   ", binstr);
			}
			sprintf(binstr, "%s  ", binstr);
			k = 16 - k;
			for (j = i - k; j < i; j++) {
				sprintf(binstr, "%s%c", binstr, ('!' < buf[j] && buf[j] <= '~') ? buf[j] : '.');
			}
			printf("%s\n", binstr);
		}
	}


	MysqlDriver::MysqlDriver()
	{
		m_pMysql = NULL;
	}

	MysqlDriver::~MysqlDriver()
	{
		cout << "~MysqlDriver:m_pMysql" << m_pMysql << endl;
		if (m_pMysql)
			close_mysql();
		m_pMysql = NULL;
	}

	int MysqlDriver::mysql_connect(const char *host, const char *username, const char *password,
		const char *database, int port, const char * unixsocket = NULL, int flag = 0)
	{
		if (mysql_library_init(0, NULL, NULL) != 0)
		{
			cout << "sql init library error!" << endl;
			return -1;
		}
		m_pMysql = mysql_init(NULL);
		if (m_pMysql == NULL)
		{
			cout << "sql mysql_init error :" << mysql_error(m_pMysql) << endl;
			return -1;
		}
		if (0 != mysql_options(m_pMysql, MYSQL_SET_CHARSET_NAME, "utf8mb4"))
		{
			cout << "sql MYSQL_SET_CHARSET_NAME error :" << mysql_error(m_pMysql) << endl;
			return -1;
		}
		char cValue = 1;
		if (0 != mysql_options(m_pMysql, MYSQL_OPT_RECONNECT, (char*)&cValue))
		{
			cout << "sql MYSQL_OPT_RECONNECT error :" << mysql_error(m_pMysql) << endl;
			return -1;
		}

		if (mysql_real_connect(m_pMysql, host, username, password, database, port, unixsocket, flag) == NULL)
		{
			int iErrorCode = mysql_errno(m_pMysql);
			//unknown database,maybe new db
			cout << "connect errorcode=" << iErrorCode << ",sql error :" << mysql_error(m_pMysql) << endl;
			return -1 * iErrorCode;
		}
		return 0;
	}


	int64_t MysqlDriver::mysql_exec(const char *sql, Call_back  call_back, void *param)
	{
		int64_t ret = 0;

		if (!strncmp(sql, "select", 6) || !strncmp(sql, "SELECT", 6))
		{
			ret = mysql_select(sql, call_back, param);
		}
		else
		{
			ret = do_sql(sql);
		}
		return ret;
	}

	//real excute sql statement
	int64_t MysqlDriver::do_sql(const char *sql, Call_back  call_back, void *param)
	{
		int64_t ret = 0;
		MYSQL_RES *res;
		MYSQL_ROW row;

		high_resolution_clock::time_point beginTime = high_resolution_clock::now();
		ret = mysql_real_query(m_pMysql, sql, strlen(sql));
		if (ret)
		{
			//todo : error code 2013,lost connect
			if (2013 == ret)
			{
				ret = mysql_real_query(m_pMysql, sql, strlen(sql));
				cout << "query again! " << endl;
			}
			if (ret)
			{
				//show error msg��
				cout << "error errcode=" << mysql_errno(m_pMysql) << ",sql:" << sql << endl;
				cout << "Error mysql_real_query failed,ret=" << ret << ", error:" << mysql_error(m_pMysql) << endl;
				return -1 * ret;
			}
		}
		high_resolution_clock::time_point endTime = high_resolution_clock::now();
		milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(endTime - beginTime);
		//cout << "mysql_real_query cost time=" << timeInterval.count() << " ms" << endl;


		do {
			res = mysql_store_result(m_pMysql);
			int n_column = mysql_field_count(m_pMysql);
			
			//must (select、show，，，，),/
			if (res )
			{
				row = mysql_fetch_row(res);
				unsigned long *lengths = mysql_fetch_lengths(res);
				//todo: error condition1
				//excute replace statement,mysql connect closed by unexpect reason 。so that res is no null、n_column=2
				if (call_back)
				{
					if (!row)
						call_back(row, n_column, lengths, param);
					else
					{
						while (row)
						{
							call_back(row, n_column, lengths, param);
							row = mysql_fetch_row(res);
						}
					}
				}
				else
				{
					LOG_ERROR("sql:%s,\nHave res but no call_back,err_str=%s ", sql,  mysql_error(m_pMysql));
					mysql_free_result(res);
					return -1 ;
				}
				ret = mysql_num_rows(res);
				mysql_free_result(res);

			}
			else //should be insert/replace/update/delete and so on
			{
				if (n_column == 0)
				{//get affected row
					ret = mysql_affected_rows(m_pMysql);
					return ret;
				}
				else
				{
					cout << "Error mysql_store_result n_column failed. error:" << mysql_error(m_pMysql) << endl;
					return -1 * n_column;
				}
			}
		} while (mysql_next_result(m_pMysql) == 0);

		return ret;
	}


	int64_t MysqlDriver::mysql_select(const char *sql, Call_back  call_back, void *param)
	{
		if (NULL == call_back)
		{
			cout << "mysql_select error :call_back is NULL!" << endl;
			return -1;
		}
		int64_t iRet = do_sql(sql, call_back, param);
		if (iRet < 0)
		{
			cout << "mysql_select error sql:" << sql << "。 error reason:" << mysql_error(m_pMysql) << endl;
		}
		return iRet;
	}




	void MysqlDriver::close_mysql()
	{
		mysql_close(m_pMysql);
		m_pMysql = NULL;
	}

	int64_t MysqlDriver::select_db(const char*db_name)
	{
		return mysql_select_db(m_pMysql, db_name);
	}

	long MysqlDriver::format_blob_string(char *sv_str, const char *orgStr, int iLen)
	{
		return mysql_real_escape_string(m_pMysql, sv_str, orgStr, iLen);
	}


	/***********   tidb *******************/

	tidb::tidb(const std::string host_ip, const std::string user_name, const std::string pwd, int32_t port)
	{
		m_Host_ip = host_ip;
		m_User_name = user_name;
		m_Password = pwd;
		m_Port = port;
		m_pMysqlDriver = new MysqlDriver();
		initialize();
	}

	tidb::~tidb()
	{
		if (m_pMysqlDriver)
		{
			delete m_pMysqlDriver;
		}
		m_pMysqlDriver = NULL;
	}

	bool tidb::initialize()
	{
		//todo: for some init
		return true;
	}

	bool tidb::initDB(const char *db_name)
	{
		std::string sql_str = "CREATE DATABASE IF NOT EXISTS ";
		sql_str.append(db_name);
		sql_str.append(" character set utf8mb4;");


		int iRet = m_pMysqlDriver->mysql_connect(m_Host_ip.c_str(), m_User_name.c_str(), m_Password.c_str(), NULL, m_Port);
		if (0 != iRet)
			return false;

		if (m_pMysqlDriver->mysql_exec(sql_str.c_str()) < 0)
			return false;
		return true;
	}


	bool tidb::create_kv_table()
	{

		const char* sql = "CREATE TABLE IF NOT EXISTS kv_table ( \
						  						  		id BIGINT UNSIGNED  PRIMARY KEY AUTO_INCREMENT ,\
																								kv_key VARCHAR(255) NOT NULL UNIQUE  ,\
																																				kv_data MEDIUMBLOB ,\
																																																		index idx_key(kv_key))  \
																																																																		ENGINE = InnoDB DEFAULT CHARACTER SET utf8mb4;";

		
		if (m_pMysqlDriver->mysql_exec(sql) < 0)
			return false;
		return true;
	}


	bool tidb::Open(const std::string &db_name)
	{

		int64_t iRet = m_pMysqlDriver->mysql_connect(m_Host_ip.c_str(), m_User_name.c_str(), m_Password.c_str(), db_name.c_str(), m_Port);
		if (0 != iRet)
		{
			//unknown database,new db
			if (-1049 == iRet)
			{
				//create TIDB_DB_NAME.
				//todo:init more database
				if (initDB(db_name.c_str()))
				{
					iRet = m_pMysqlDriver->select_db(db_name.c_str());
					create_kv_table();
					return true;
				}

			}
			return false;
		}

		return true;

	}


	bool tidb::Close()
	{
		m_pMysqlDriver->close_mysql();
		return true;
	}

	int64_t tidb::Get(const std::string &key, std::string &value)
	{
		int64_t time_start = utils::Timestamp::HighResolution();
		int iLen = 100 + (key.length() * 2 * 8 + 1);
		char *sql = new char[iLen];
		if (!sql)
		{
			cout << "tidb.cpp new char error,not enough size=" << iLen << endl;
			return false;
		}
		memset(sql, 0x00, 100);

		char *end = strcpy(sql, "select kv_key,kv_data from kv_table where kv_key=TO_BASE64(\'");
		end += strlen(sql);
		end += m_pMysqlDriver->format_blob_string(end, key.c_str(), key.length());
		*end++ = '\'';
		*end++ = ')';
		*end++ = 0x00;


		int64_t iRet = m_pMysqlDriver->mysql_exec(sql, get_table_value, (void *)&value);
		delete[]sql;

		int64_t time_use = utils::Timestamp::HighResolution() - time_start;

		return iRet;
	}

	bool tidb::Put(const std::string &key, const std::string &value)
	{
		//replace statement require   have >=1 unique/primary column��otherwise insert(ignore update) 
		/*kv_data <= 16M ,but tidb:single KV entry <= 6MB*/
		int64_t time_start = utils::Timestamp::HighResolution();

		//allocate space
		int iLen = 100 + (key.length() * 2 * 8 + 1) + (value.length() * 2 + 1);
		char *sql = new char[iLen];
		if (!sql)
		{
			cout << "tidb.cpp new char error,not enough size=" << iLen << endl;
			return false;
		}
		memset(sql, 0x00, 100);

		char *end = strcpy(sql, "replace into kv_table(kv_key,kv_data) values (TO_BASE64(\'");
		end += strlen(sql);
		end += m_pMysqlDriver->format_blob_string(end, key.c_str(), key.length());
		*end++ = '\'';
		*end++ = ')';
		*end++ = ',';
		*end++ = '\'';
		end += m_pMysqlDriver->format_blob_string(end, value.c_str(), value.length());
		*end++ = '\'';
		*end++ = ')';
		*end++ = 0x00;

		int64_t iRet = m_pMysqlDriver->mysql_exec(sql);
		delete[]sql;

		int64_t time_use = utils::Timestamp::HighResolution() - time_start;
		LOG_INFO("Put one key use time(" FMT_I64 "ms)", (int64_t)(time_use / utils::MILLI_UNITS_PER_SEC));
		if (iRet < 0)
			return false;
		return true;
	}

	bool tidb::Put(WriteTidbBatch &value)
	{
		//todo: real write batch
		std::map<std::string, std::string> batch_map = value.get_put_map();
		std::string sql_str;
		bool bRet = true;
		for (std::map<std::string, std::string>::iterator it = batch_map.begin(); it != batch_map.end(); ++it)
		{
			Put(it->first, it->second);
		}

		//todo: do real batch
		return bRet;
	}

	bool tidb::Delete(const std::string &key)
	{
		int iLen = 36 + (key.length() * 2 + 1);
		char *sql = new char[iLen];
		if (!sql)
		{
			cout << "tidb.cpp new char error,not enough size=" << iLen << endl;
			return false;
		}
		memset(sql, 0x00, 100);

		char *end = strcpy(sql, "delete from kv_table where kv_key=\'");
		end += strlen(sql);
		end += m_pMysqlDriver->format_blob_string(end, key.c_str(), key.length());
		*end++ = '\'';
		*end++ = 0x00;

		int64_t iRet = m_pMysqlDriver->mysql_exec(sql);
		delete[]sql;

		if (iRet < 0)
			return false;
		return true;
	}



}