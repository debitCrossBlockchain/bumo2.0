
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

#define M_UNIT					1024*1024
#define SPILT_TRANS_SIZE		(5*M_UNIT)
#define SINGLE_ENTRY_SIZE		6*M_UNIT
#define TRANS_SIZE_LIMIT		100*M_UNIT
#define TRANS_NUMBER_LIMIT		30*10000


#define REPLACE_STMT_SQL	"replace into kv_table(kv_key,kv_data) values (?,?)"

#define SELECT_STMT_SQL		"select kv_key,kv_data from kv_table where kv_key=? "

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

	void get_all_data(MYSQL_ROW row, int n_column, unsigned long * len, void * param)
	{

		if (row)
		{
			std::map<string, string> *out_map = (std::map<string, string> *)param;
			string str_data;
			str_data.append(IsNull(row[1]), len[1]);
			(*out_map)[string(IsNull(row[0]))] = str_data;
			//out_map->insert(std::make_pair(str_key, str_data));
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
		for (int i = 0; i < STMT_NUM; i++)
			stmt[i] = NULL;
	}

	MysqlDriver::~MysqlDriver()
	{
		//LOG_ERROR( "~MysqlDriver:m_pMysql" << m_pMysql );
		//mysql_stmt_close(stmt_i);
		for (int i = 0; i < STMT_NUM; i++)
		{
			mysql_stmt_close(stmt[i]);
			stmt[i] = NULL;
		}
		if (m_pMysql)
			close_mysql();
		m_pMysql = NULL;

	}

	std::string MysqlDriver::get_err_str()
	{
		char err_code[256];
#ifdef WIN32
		_snprintf(err_code, 256, "error_code:%d,error_string:%s\n", mysql_errno(m_pMysql), mysql_error(m_pMysql));
		err_code[255] = '\0';
#else
		snprintf(err_code, 256, "error_code:%d,error_string:%s\n", mysql_errno(m_pMysql), mysql_error(m_pMysql));
#endif
		std::string ret_str = err_code;
		return ret_str;
	}

	int MysqlDriver::mysql_connect(const char *host, const char *username, const char *password,
		const char *database, int port, const char * unixsocket = NULL, int flag = 0)
	{
		if (mysql_library_init(0, NULL, NULL) != 0)
		{
			LOG_ERROR( "sql init library error!" );
			return -1;
		}
		m_pMysql = mysql_init(NULL);
		if (m_pMysql == NULL)
		{
			LOG_ERROR( "sql mysql_init error :%s" , mysql_error(m_pMysql) );
			return -1;
		}
		if (0 != mysql_options(m_pMysql, MYSQL_SET_CHARSET_NAME, "utf8mb4"))
		{
			LOG_ERROR( "sql MYSQL_SET_CHARSET_NAME error :%s" , mysql_error(m_pMysql) );
			return -1;
		}
		char cValue = 1;
		if (0 != mysql_options(m_pMysql, MYSQL_OPT_RECONNECT, (char*)&cValue))
		{
			LOG_ERROR( "sql MYSQL_OPT_RECONNECT error :%s" , mysql_error(m_pMysql) );
			return -1;
		}

		if (mysql_real_connect(m_pMysql, host, username, password, database, port, unixsocket, flag) == NULL)
		{
			int iErrorCode = mysql_errno(m_pMysql);
			//unknown database,maybe new db
			if (iErrorCode == 1049)
				LOG_WARN("WARN string :%s", mysql_error(m_pMysql));
			else
				LOG_ERROR("connect errorcode=%d ,sql error :%s" ,iErrorCode,mysql_error(m_pMysql));
			return -1 * iErrorCode;
		}
		//close autocommit
		mysql_autocommit(m_pMysql, 0);
		return 0;
	}

	bool MysqlDriver::init_stmt(const char* stmt_sql, int iPos)
	{
		if (stmt[iPos])
			mysql_stmt_close(stmt[iPos]);
		//init stmt
		stmt[iPos] = mysql_stmt_init(m_pMysql);
		if (!stmt[iPos])
		{
			int iErrorCode = mysql_errno(m_pMysql);
			//unknown database,maybe new db
			LOG_ERROR("mysql_stmt_init errorcode=%d ,error :%s"  , iErrorCode,mysql_error(m_pMysql));
			return false;
		}
		if (mysql_stmt_prepare(stmt[iPos], stmt_sql, strlen(stmt_sql)))
		{
			LOG_ERROR( "mysql_stmt_prepare  error :%s" , mysql_error(m_pMysql) );
			return false;
		}
		return true;
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

		int64_t time_start = utils::Timestamp::HighResolution();
		ret = mysql_real_query(m_pMysql, sql, strlen(sql));
		mysql_commit(m_pMysql);
		if (ret)
		{
			//todo : error code 2013,lost connect
			if (2013 == ret)
			{
				ret = mysql_real_query(m_pMysql, sql, strlen(sql));
				LOG_ERROR( "query again! " );
			}
			if (ret)
			{
				//show error msg��
				LOG_ERROR("error errcode=%d ,sql:%s"  , mysql_errno(m_pMysql),sql);
				LOG_ERROR("Error mysql_real_query failed,ret=%d , error:%s"  , ret,mysql_error(m_pMysql));
				return -1 * ret;
			}
		}

		int64_t time_use = utils::Timestamp::HighResolution() - time_start;

		do {
			res = mysql_store_result(m_pMysql);
			int n_column = mysql_field_count(m_pMysql);

			//must (select、show，，，，),/
			if (res)
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
							lengths = mysql_fetch_lengths(res);
						}
					}
				}
				else
				{
					LOG_ERROR("sql:%s,Have res but no call_back,err_str=%s ", sql, mysql_error(m_pMysql));
					mysql_free_result(res);
					return -1;
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
					LOG_ERROR( "Error mysql_store_result n_column failed. error:%s" , mysql_error(m_pMysql) );
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
			LOG_ERROR( "mysql_select error :call_back is NULL!" );
			return -1;
		}
		int64_t iRet = do_sql(sql, call_back, param);
		if (iRet < 0)
		{
			LOG_ERROR("mysql_select error sql:%s。 error reason:%s" ,sql,mysql_error(m_pMysql));
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

	int32_t MysqlDriver::do_commit()
	{
		return mysql_commit(m_pMysql);
	}

	int32_t MysqlDriver::roll_back()
	{
		return mysql_rollback(m_pMysql);
	}

	int64_t MysqlDriver::stmt_exec(MYSQL_BIND *param, int stmt_series)
	{
		MYSQL_STMT * _stmt = stmt[stmt_series];
		if (!param || !_stmt)
		{
			LOG_ERROR( "stmt_exec param/stmt is NULL" );
			return -1;
		}

		if (mysql_stmt_bind_param(_stmt, param))
		{
			LOG_ERROR( "mysql_stmt_bind_param  error :%s" , mysql_stmt_error(_stmt) );
			return -1;
		}

		if (mysql_stmt_execute(_stmt))
		{
			LOG_ERROR( "mysql_stmt_execute  error :%s" , mysql_stmt_error(_stmt) );
			return -1;
		}

		return 0;
	}

	/**************************************
	***************************************
	***************************************
	***************************************
	**********  TIDB  *********************
	***************************************
	***************************************
	***************************************
	***************************************
	**********************************/

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
					return init_stmt();
				}

			}
			return false;
		}

		return init_stmt();

	}

	bool tidb::init_stmt()
	{
		//bool bRet = false;
		std::string stmt_sql = REPLACE_STMT_SQL;
		for (int i = 0; i < REPLACE_BATCH_NUM; i++)
		{
			if (!m_pMysqlDriver->init_stmt(stmt_sql.c_str(), i))
				return false;
			stmt_sql += ",(?,?)";
		}
//		if (!m_pMysqlDriver->init_stmt(SELECT_STMT_SQL,10))
//			return false;
		return true;
	}


	bool tidb::Close()
	{
		m_pMysqlDriver->close_mysql();
		return true;
	}
	
	int64_t tidb::Get(const std::string &key, std::string &value)
	{

		std::string tmpStr = "select kv_key,kv_data from kv_table where kv_key='";
		tmpStr += utils::encode_b64(key);
		tmpStr += "'";

		int64_t time_start = utils::Timestamp::HighResolution();
		int64_t iRet = m_pMysqlDriver->mysql_exec(tmpStr.c_str(), get_table_value, (void *)&value);
		//int64_t iRet = m_pMysqlDriver->mysql_select2(utils::encode_b64(key).c_str(), get_table_value, (void *)&value);
		int64_t time_use = utils::Timestamp::HighResolution() - time_start;
		LOG_INFO("Get_Mysql key use time( " FMT_I64 " ms)", (int64_t)(time_use / utils::MILLI_UNITS_PER_SEC));


		return iRet;
	}
	


	bool tidb::Put(const std::string &key, const std::string &value)
	{
		//replace statement require   have >=1 unique/primary column��otherwise insert(ignore update) 
		/*kv_data <= 16M ,but tidb:single KV entry <= 6MB*/
		int64_t time_start = utils::Timestamp::HighResolution();
		int64_t iRet = -1;

		std::string tmpStr = utils::encode_b64(key);
		iRet = do_replace_stmt(tmpStr, value);
		int64_t time_use = utils::Timestamp::HighResolution() - time_start;
		LOG_INFO("Put_one key use time( " FMT_I64 " ms)", (int64_t)(time_use / utils::MILLI_UNITS_PER_SEC));
		if (iRet < 0)
			return false;
		return true;
	}

	bool tidb::Put(WriteTidbBatch &value)
	{
		//todo: real write batch
		int64_t time_start = utils::Timestamp::HighResolution();
		std::map<std::string, std::string> batch_map = value.get_put_map();
		bool bRet = true;

		int64_t iRet = do_replace_stmt(batch_map);
		int64_t time_use = utils::Timestamp::HighResolution() - time_start;

		LOG_INFO("WriteTidbBatch use time( " FMT_I64 " ms),sql count=%d. data_len=%ld", (int64_t)(time_use / utils::MILLI_UNITS_PER_SEC), batch_map.size(), iRet);
		//todo: do real batch
		if (iRet < 0)
			return false;
		return bRet;
	}

	bool tidb::Delete(const std::string &key)
	{
		int iLen = 36 + (key.length() * 2 + 1);
		char *sql = new char[iLen];
		if (!sql)
		{
			LOG_ERROR( "tidb.cpp new char error,not enough size=%d" , iLen );
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

	int64_t tidb::Get_All_Ledger(std::map<std::string, std::string> &_out_map)
	{
		int64_t time_start = utils::Timestamp::HighResolution();

		const char *sql = "select kv_key,kv_data from kv_table;";

		int64_t iRet = m_pMysqlDriver->mysql_exec(sql, get_all_data, (void*)&_out_map);

		int64_t time_use = utils::Timestamp::HighResolution() - time_start;

		return iRet;

	}
	
	int64_t tidb::do_replace_stmt(std::map<std::string, std::string> &in_map)
	{
		if (in_map.size() <= 0)
			return 0;
		int64_t iBatchLen = 0;
		int iSeq = 0;
		int iCommitlen = strlen(REPLACE_STMT_SQL)+7*9;

		MYSQL_BIND params[REPLACE_BATCH_NUM*2];
		memset(params, 0, sizeof(params));

		std::map<std::string, std::string>::iterator iter = in_map.begin();
		int i = 0;
		std::string tmpStr[REPLACE_BATCH_NUM * 2];
		while (iter != in_map.end())
		{
			tmpStr[i] = utils::encode_b64(iter->first);
			//replace error about this data > 6M
			if (tmpStr[i].length() + iter->second.length() > SINGLE_ENTRY_SIZE)
			{
				LOG_ERROR("data:%s length more than 6M ,len=%d!", iter->first.c_str(), tmpStr[i].length() + iter->second.length());
				//how to do?
				/*
				//1.commit previous stmt
				if (i != 0 && m_pMysqlDriver->stmt_exec(params, (i - 1) / 2) < 0)
				{
					m_pMysqlDriver->roll_back();
					LOG_ERROR("stmt_exec error!");
					return -1;
				}
				if (0 != m_pMysqlDriver->do_commit())
				{
					m_pMysqlDriver->roll_back();
					LOG_ERROR("do_commit error:%s", m_pMysqlDriver->get_err_str());
					return -1;
				}
				//2.split this row
				*/
			}
			//kv entry limit 100M
			if (iCommitlen + tmpStr[i].length() + iter->second.length() > TRANS_SIZE_LIMIT)
			{
				if (i!=0 && m_pMysqlDriver->stmt_exec(params, (i-1)/ 2 ) < 0)
				{
					m_pMysqlDriver->roll_back();
					LOG_ERROR("stmt_exec error!");
					return -1;
				}
				if (0 != m_pMysqlDriver->do_commit())
				{
					m_pMysqlDriver->roll_back();
					LOG_ERROR("do_commit error:%s", m_pMysqlDriver->get_err_str().c_str());
					return -1;
				}
				tmpStr[0] = tmpStr[i];
				i = 0;
				iBatchLen += iCommitlen;
				iCommitlen = strlen(REPLACE_STMT_SQL) + 7 * 9;
				memset(params, 0, sizeof(params));
			}

			params[i].buffer_type = MYSQL_TYPE_STRING;
			params[i].buffer = (void *)tmpStr[i].c_str();
			params[i].buffer_length = tmpStr[i].length();
			iCommitlen += params[i].buffer_length;
			i++;
			params[i].buffer_type = MYSQL_TYPE_MEDIUM_BLOB;
			params[i].buffer = (void *)iter->second.c_str();
			params[i].buffer_length = iter->second.length();
			iCommitlen += params[i].buffer_length;
			i++;

			iter++;
			//collect REPLACE_BATCH_NUM params set. or at the end .now excute sql
			if (i == REPLACE_BATCH_NUM * 2  || iter == in_map.end())
			{
				if (m_pMysqlDriver->stmt_exec(params, (i - 1) / 2) < 0)
				{
					m_pMysqlDriver->roll_back();
					m_pMysqlDriver->do_commit();
					LOG_ERROR("stmt_exec error!");
					return -1;
				}
				i = 0;
				memset(params, 0, sizeof(params));
			}
		
		} 
		if (0 != m_pMysqlDriver->do_commit())
		{
			LOG_ERROR("do_commit error:%s", m_pMysqlDriver->get_err_str().c_str());
			return -1;
		}
		iBatchLen += iCommitlen;
		return iBatchLen;
	}

	int64_t tidb::do_replace_stmt(const std::string &key, const std::string &value)
	{
		MYSQL_BIND params[2];
		memset(params, 0, sizeof(params));

		params[0].buffer_type = MYSQL_TYPE_STRING;
		params[0].buffer = (void *)key.c_str();
		params[0].buffer_length = key.length();

		params[1].buffer_type = MYSQL_TYPE_MEDIUM_BLOB;
		params[1].buffer = (void *)value.c_str();
		params[1].buffer_length = value.length();


		if (m_pMysqlDriver->stmt_exec(params, 0) < 0)
		{
			LOG_ERROR( "stmt_exec error!" );
			return -1;
		}
		if (0 != m_pMysqlDriver->do_commit())
		{
			LOG_ERROR( "do_commit error!" );
			return -1;
		}

		return 0;
	}


	


}