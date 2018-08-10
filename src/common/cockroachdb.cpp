
#include "cockroachdb.h"
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

#define UPSERT_STMT_SQL	    "upsert into kv_table(kv_key,kv_data) values ($1,$2),($3,$4),($5,$6),($7,$8),($9,$10),\
							($11,$12),($13,$14),($15,$16),($17,$18),($19,$20)"


#define UPSERT_STMT_SQL_SINGLE	    "upsert into kv_table(kv_key,kv_data) values ($1,$2)"

#define UPSERT_STMT_SQL_DEMO	    "upsert into kv_table(kv_key,kv_data) values "

	string g_upset_stmt[10] = { "upsert_1", "upsert_2", "upsert_3", "upsert_4", "upsert_5", "upsert_6", "upsert_7", "upsert_8", "upsert_9", "upsert_10" };







	/**************************************
	***************************************
	***************************************
	***************************************
	**********  codb  *********************
	***************************************
	***************************************
	***************************************
	***************************************
	**********************************/

	codb::codb(const std::string host_ip, const std::string user_name, const std::string pwd, int32_t port)
	{
		m_Host_ip = host_ip;
		m_User_name = user_name;
		m_Password = pwd;
		m_Port = port;

		initialize();
	}

	codb::~codb()
	{
		if (c1)
		{
			delete c1;
		}
		c1 = NULL;
		if (c2)
		{
			delete c2;
		}
		c2 = NULL;
	}

	bool codb::initialize()
	{
		//todo: for some init
		return true;
	}



	bool codb::create_kv_table()
	{

		std::string sql = "CREATE TABLE IF NOT EXISTS kv_table ( \
						  						  						 						kv_key VARCHAR(255) PRIMARY KEY NOT NULL UNIQUE, \
																																																												kv_data BYTES, \
																																																																																																																								index idx_key(kv_key)); ";
		try{
			pqxx::work w(*c1);
			w.exec(sql);
			w.commit();
		}
		catch (const exception &e) {
			cerr << e.what() << endl;
			return false;
		}
		return true;
	}


	bool codb::Open(const std::string &db_name)
	{
		char conn_param[128];
		try{
			//sprintf(conn_param, "dbname=%s user=%s hostaddr=%s port=%d\0", db_name.c_str(), m_User_name.c_str(), m_Host_ip.c_str(), m_Port);
			sprintf(conn_param, "dbname=%s user=codb hostaddr=127.0.0.1 port=26257\0", db_name.c_str());
			c1 = new pqxx::connection(conn_param);
			c2 = new pqxx::connection(conn_param);
			if (!c1->is_open() || !c2->is_open())
				LOG_ERROR("connection open error!");
			c1->set_client_encoding("UTF8");
			c2->set_client_encoding("UTF8");

		}
		catch (const exception &e) {
			cerr << "Open error:" << e.what() << " param=" << conn_param << endl;
			return false;
		}
		//todo:  init databases and table
		/*
		//create database
		CREATE DATABASE IF NOT EXISTS LEDGERDB ENCODING = 'UTF-8'; CREATE DATABASE IF NOT EXISTS KVDB ENCODING = 'UTF-8'; CREATE DATABASE IF NOT EXISTS ACCOUNTDB ENCODING = 'UTF-8';
		//create table
		use LEDGERDB;CREATE TABLE IF NOT EXISTS kv_table ( kv_key VARCHAR(255) PRIMARY KEY NOT NULL UNIQUE  ,kv_data BYTES,index idx_key(kv_key)); use KVDB;CREATE TABLE IF NOT EXISTS kv_table ( kv_key VARCHAR(255) PRIMARY KEY NOT NULL UNIQUE  ,kv_data BYTES,index idx_key(kv_key));use ACCOUNTDB;CREATE TABLE IF NOT EXISTS kv_table ( kv_key VARCHAR(255) PRIMARY KEY NOT NULL UNIQUE  ,kv_data BYTES,index idx_key(kv_key));
		//privileges
		use LEDGERDB;GRANT ALL ON table kv_table TO codb; use KVDB;GRANT ALL ON table kv_table TO codb; use ACCOUNTDB;GRANT ALL ON table kv_table TO codb;

		*/
		create_kv_table();
		return init_stmt();

	}

	bool codb::init_stmt()
	{
		//bool bRet = false;
		c1->prepare("upset_db", UPSERT_STMT_SQL);
		c2->prepare("upset_db", UPSERT_STMT_SQL);
		c1->prepare("s_upset_db", UPSERT_STMT_SQL_SINGLE);
		c2->prepare("s_upset_db", UPSERT_STMT_SQL_SINGLE);
		//UPSERT_STMT_SQL_DEMO
		int iRow = 10;
		char pre_str[128];

		memset(pre_str, 0x00, 128);


		std::string temp_str = UPSERT_STMT_SQL_DEMO;
		for (int i = 1; i < 20; i++)
		{
			sprintf(pre_str, "%s ($%d,$%d) \0", temp_str.c_str(), i,i+1);
			i++;
			temp_str.assign(pre_str);
			c1->prepare(g_upset_stmt[i / 2 - 1], pre_str);
			c2->prepare(g_upset_stmt[i / 2 - 1], pre_str);

			temp_str.append(",");
		}
		return true;
	}



	bool codb::Close()
	{
		if (c1)
			delete c1;
		if (c2)
			delete c2;
		c1 = nullptr;
		c2 = nullptr;
		return true;
	}

	int64_t codb::Get(const std::string &key, std::string &value)
	{

		std::string tmpStr = "select kv_key,kv_data from kv_table where kv_key='";
		tmpStr += utils::encode_b64(key);
		tmpStr += "'";
		int iRet = 0;
		//int64_t time_start = utils::Timestamp::HighResolution();
		//int64_t iRet = m_pMysqlDriver->mysql_exec(tmpStr.c_str(), get_table_value, (void *)&value);

		try{
			pqxx::work w(*c1);
			pqxx::result r = w.exec(tmpStr);
			w.commit();
			iRet = r.size();
			for (auto row : r) {
				//cout << row[0].as<string>() << ' ' << row[1].as<string>() << endl;
				//value = row[1].as<string>();
				pqxx::binarystring bin(row[1]);
				value = bin.str();
			}
		}
		catch (const exception &e) {
			LOG_ERROR("codb_get error:%s", e.what());
			return -1;
		}

		return iRet;
	}



	bool codb::Put(const std::string &key, const std::string &value)
	{
		//replace statement require   have >=1 unique/primary column£¬otherwise insert(ignore update) 
		/*kv_data <= 16M ,but codb:single KV entry <= 6MB*/
		int64_t time_start = utils::Timestamp::HighResolution();
		int64_t iRet = -1;


		std::string tmpStr = utils::encode_b64(key);

		try{
			pqxx::work w(*c1);
			pqxx::prepare::invocation inv = w.prepared("s_upset_db");
			inv(tmpStr);
			inv(pqxx::binarystring(value));
			pqxx::result r = inv.exec();
			iRet = r.affected_rows();
			if (iRet == 1)
			{
				w.commit();
			}
			else
				LOG_INFO("Put_one key r.affected_rows() != 1");

		}
		catch (const exception &e) {
			LOG_ERROR("codb error:%s", e.what());
			return -1;
		}

		//iRet = do_replace_stmt(tmpStr, value);
		int64_t time_use = utils::Timestamp::HighResolution() - time_start;
		LOG_INFO("Put_one key use time( " FMT_I64 " ms)", (int64_t)(time_use / utils::MILLI_UNITS_PER_SEC));
		if (iRet < 0)
			return false;
		return true;
	}

	struct tempStu
	{
		pqxx::connection *c;
		std::map<std::string, std::string> *p_map;
	};

	void * do_upsert_stmt1(void * st)
	{
		tempStu *temp_st = (tempStu *)st;
		std::map<std::string, std::string> *in_map = temp_st->p_map;
		pqxx::connection *c = temp_st->c;

		if (in_map->size() <= 0)
			return ((void *)0);
		int iSize = (1+in_map->size()) / 2;

		int64_t iBatchLen = 0;
		int iCount = 0;

		std::map<std::string, std::string>::iterator iter = in_map->begin();
		int i = 0;

		int64_t time_start1 = utils::Timestamp::HighResolution();
		try{
			pqxx::work* w = new pqxx::work(*c);

			pqxx::prepare::invocation *inv = NULL;

			if (iSize < 10)
			{
				pqxx::prepare::invocation temp_inv = w->prepared(g_upset_stmt[iSize - 1]);
				inv = &temp_inv;
			}
			//	inv = &w->prepared(g_upset_stmt[iSize - 1]);
			else
				//	inv = &w->prepared(g_upset_stmt[9]);
			{
				pqxx::prepare::invocation temp_inv = w->prepared(g_upset_stmt[9]);
				inv = &temp_inv;
			}

			while (iSize--)
			{
				//tmpStr[i] = utils::encode_b64(iter->first);

				(*inv)(utils::encode_b64(iter->first).c_str());
				(*inv)(pqxx::binarystring(iter->second));
				//std::string temp_str = pqxx::to_string(iter->second);
				//(*inv)(utils::encode_b64(iter->first).c_str())(temp_str.c_str());
				iter++;
				i++;
				
				//collect REPLACE_BATCH_NUM params set. or at the end .now excute sql
				if (i == REPLACE_BATCH_NUM || iter == in_map->end() || iSize == 0)
				{
					pqxx::result r = inv->exec();
					iBatchLen += r.affected_rows();
					w->commit();
					delete w;
					w = new pqxx::work(*c);
					if (iSize != 0)
					{
						if (iSize < 10)
						{
							pqxx::prepare::invocation temp_inv = w->prepared(g_upset_stmt[iSize - 1]);
							inv = &temp_inv;
						}
						else
						{
							pqxx::prepare::invocation temp_inv = w->prepared(g_upset_stmt[9]);
							inv = &temp_inv;
						}
					}
					i = 0;

				}

			}
			if (w)
				delete w;
		}
		catch (const exception &e) {
			LOG_ERROR("codb error:%s", e.what());
			return ((void *)0);
		}

		int64_t time_use = utils::Timestamp::HighResolution() - time_start1;
		//LOG_INFO("WriteTidbBatch_END use time( " FMT_I64 " ms) size=%d,do_count=%d", (int64_t)(time_use / utils::MILLI_UNITS_PER_SEC), in_map->size(), iBatchLen);

		return ((void *)iBatchLen);

	}

	void * do_upsert_stmt2(void * st)
	{
		tempStu *temp_st = (tempStu *)st;
		std::map<std::string, std::string> *in_map = temp_st->p_map;
		pqxx::connection *c = temp_st->c;

		int iSize = in_map->size() - (1+in_map->size()) / 2;
		if (in_map->size() <= 0 || iSize == 0)
			return ((void *)0);
		int64_t iBatchLen = 0;
		int iCount = 0;

		std::map<std::string, std::string>::reverse_iterator riter = in_map->rbegin();
		int i = 0;

		int64_t time_start1 = utils::Timestamp::HighResolution();

		try{
			pqxx::work* w = new pqxx::work(*c);
			pqxx::prepare::invocation *inv = NULL;
			if (iSize < 10)
			{
				pqxx::prepare::invocation temp_inv = w->prepared(g_upset_stmt[iSize - 1]);
				inv = &temp_inv;
			}
			else
			{
				pqxx::prepare::invocation temp_inv = w->prepared(g_upset_stmt[9]);
				inv = &temp_inv;
			}

			while (iSize--)
			{
				//tmpStr[i] = utils::encode_b64(iter->first);

				(*inv)(utils::encode_b64(riter->first));
				(*inv)(pqxx::binarystring(riter->second));
				//(*inv)(utils::encode_b64(riter->first).c_str())(pqxx::to_string(riter->second).c_str());
				riter++;
				i++;

				if (i == REPLACE_BATCH_NUM || riter == in_map->rend() || iSize == 0)
				{
					pqxx::result r = (*inv).exec();
					iBatchLen += r.affected_rows();
					w->commit();
					delete w;
					w = new pqxx::work(*c);
					if (iSize != 0)
					{
						if (iSize < 10)
						{
							pqxx::prepare::invocation temp_inv = w->prepared(g_upset_stmt[iSize - 1]);
							inv = &temp_inv;
						}
						else
						{
							pqxx::prepare::invocation temp_inv = w->prepared(g_upset_stmt[9]);
							inv = &temp_inv;
						}
					}
					i = 0;

				}

			}
			if (w)
				delete w;
		}
		catch (const exception &e) {
			LOG_ERROR("codb error:%s", e.what());
			return ((void *)0);
		}
		int64_t time_use = utils::Timestamp::HighResolution() - time_start1;
		//LOG_INFO("WriteTidbBatch2_END use time( " FMT_I64 " ms) size=%d,do_count=%d", (int64_t)(time_use / utils::MILLI_UNITS_PER_SEC), in_map->size(), iBatchLen);

		return ((void *)iBatchLen);

	}


	bool codb::Put(WriteCodbBatch &value)
	{
		//todo: real write batch
		int64_t time_start = utils::Timestamp::HighResolution();
		std::map<std::string, std::string> batch_map = value.get_put_map();
		bool bRet = true;

		//int64_t iRet = do_replace_stmt(batch_map);
		tempStu stu_arg[2];
		stu_arg[0].c = c1;
		stu_arg[0].p_map = &batch_map;

		stu_arg[1].c = c2;
		stu_arg[1].p_map = &batch_map;

		int64_t iRet = 0;
		{
			pthread_t id1, id2;
			iRet = pthread_create(&id1, NULL, do_upsert_stmt1, (void *)&stu_arg[0]);
			if (iRet)
			{
				printf("Create pthread error!\n");
				return 1;
			}

			iRet = pthread_create(&id2, NULL, do_upsert_stmt2, (void *)&stu_arg[1]);
			if (iRet)
			{
				printf("Create pthread error!\n");
				return 1;
			}

			pthread_join(id1, NULL);
			pthread_join(id2, NULL);
		}



		int64_t time_use = utils::Timestamp::HighResolution() - time_start;

		LOG_INFO("WriteTidbBatch use time( " FMT_I64 " ms),sql count=%d. data_len=%ld", (int64_t)(time_use / utils::MILLI_UNITS_PER_SEC), batch_map.size(), iRet);
		//todo: do real batch
		//	if (iRet < 0)
		//		return false;
		return bRet;
	}

	bool codb::Delete(const std::string &key)
	{

		std::string tmpStr = "delete from kv_table where kv_key=\'";
		tmpStr += utils::encode_b64(key);
		tmpStr += "'";
		int iRet = 0;

		try{
			pqxx::work w(*c1);
			pqxx::result r = w.exec(tmpStr);
			w.commit();
			iRet = r.size();
		}
		catch (const exception &e) {
			LOG_ERROR("codb_delete error:%s", e.what());
			return false;
		}

		return true;
	}

	int64_t codb::Get_All_Ledger(std::map<std::string, std::string> &_out_map)
	{
		return 1;

	}









}