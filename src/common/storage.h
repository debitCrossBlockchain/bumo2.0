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

#ifndef STORAGE_H_
#define STORAGE_H_

#include <unordered_map>
#include <utils/headers.h>
#include <utils/sqlparser.h>
#include <json/json.h>
#include "general.h"
#include "configure_base.h"
#ifdef WIN32
#include <leveldb/leveldb.h>
#else
#include <rocksdb/db.h>
#endif
#include "tidb.h"

namespace bumo {
	/*
#ifdef WIN32
#define KVDB leveldb
#define WRITE_BATCH leveldb::WriteBatch
#define WRITE_BATCH_DATA(batch) (((std::string*)(&batch))->c_str())
#define WRITE_BATCH_DATA_SIZE(batch) (((std::string*)(&batch))->size())
#define SLICE       leveldb::Slice
#elif ROCKSDB 
#define KVDB rocksdb
#define WRITE_BATCH rocksdb::WriteBatch
#define WRITE_BATCH_DATA(batch) (batch.Data().c_str())
#define WRITE_BATCH_DATA_SIZE(batch) (batch.GetDataSize())
#define SLICE       rocksdb::Slice
#else
	*/
#define WRITE_BATCH DBBatch

//#endif

	class DBBatch
	{
	public:
		DBBatch();
		~DBBatch() {}

		static std::string s_db_type;


#ifdef WIN32
		leveldb::WriteBatch m_leveldb_batch;
#else
		rocksdb::WriteBatch m_rocksdb_batch;
#endif
		WriteTidbBatch m_tidb_batch;

		bool isTidb;

	public:
		// Store the mapping "key->value" in the database.
		void Put(const std::string& key, const std::string& value);

		void Delete(const std::string& key);

		void Clear();
	};

	class KeyValueDb {
	protected:
		utils::Mutex mutex_;
		std::string error_desc_;
	public:
		KeyValueDb();
		~KeyValueDb();
		virtual bool Open(const std::string &db_path, int max_open_files) = 0;
		virtual bool Close() = 0;
		virtual int32_t Get(const std::string &key, std::string &value) = 0;
		virtual bool Put(const std::string &key, const std::string &value) = 0;
		virtual bool Delete(const std::string &key) = 0;
		virtual bool GetOptions(Json::Value &options) = 0;
		std::string error_desc() {
			return error_desc_;
		}
		virtual bool WriteBatch(WRITE_BATCH &values) = 0;

		virtual void* NewIterator() = 0;
	};

#ifdef WIN32
	class LevelDbDriver : public KeyValueDb {
	private:
		leveldb::DB* db_;

	public:
		LevelDbDriver();
		~LevelDbDriver();

		bool Open(const std::string &db_path, int max_open_files);
		bool Close();
		int32_t Get(const std::string &key, std::string &value);
		bool Put(const std::string &key, const std::string &value);
		bool Delete(const std::string &key);
		bool GetOptions(Json::Value &options);
		bool WriteBatch(WRITE_BATCH &values);

		void* NewIterator();
	};
#else  
	class RocksDbDriver : public KeyValueDb {
	private:
		rocksdb::DB* db_;

	public:
		RocksDbDriver();
		~RocksDbDriver();

		bool Open(const std::string &db_path, int max_open_files);
		bool Close();
		int32_t Get(const std::string &key, std::string &value);
		bool Put(const std::string &key, const std::string &value);
		bool Delete(const std::string &key);
		bool GetOptions(Json::Value &options);
		bool WriteBatch(WRITE_BATCH &values);

		void* NewIterator();
	};

#endif
	class TidbDriver : public KeyValueDb {
	private:
		tidb * db_;

	public:
		TidbDriver(const std::string host_ip, const std::string user_name, const std::string pwd,int32_t port)
		{
			db_ = new tidb(host_ip,  user_name,  pwd, port);
		}
		~TidbDriver()
		{
			if (db_)
			{
				delete db_;
			}
			db_ = NULL;
		}

		bool Open(const std::string &db_name, int max_open_files)	{
			bool ret =  db_->Open(db_name);
			if(!ret)
				error_desc_ = db_->get_error();
			return ret;
		}

		bool Close()	{return db_->Close();}

		int32_t Get(const std::string &key, std::string &value)	{
			int64_t ret =  db_->Get(key,value);
			if(ret<0)
				error_desc_ = db_->get_error();
			return (int32_t)ret;
		}
		bool Put(const std::string &key, const std::string &value)	{
			bool ret =  db_->Put(key,value);
			if(!ret)
				error_desc_ = db_->get_error();
			return ret;
		}
		bool Delete(const std::string &key)		{
			bool ret =  db_->Delete(key);
			if(!ret)
				error_desc_ = db_->get_error();
			return ret;
		}

		bool DropDB(const std::string &db_name)		{
			return true;
		}
		//todo »ñÈ¡Êý¾Ý¿â×´Ì¬
		bool GetOptions(Json::Value &options)	{return true;}
		bool WriteBatch(WRITE_BATCH &values)	{
			return db_->Put(values.m_tidb_batch);
		}
		//todo ´òÓ¡ËùÓÐ¼ÇÂ¼
		void* NewIterator() {return NULL;}

		tidb * GetTiDBInstance()	{ return db_; }
	};



	

	class Storage : public utils::Singleton<bumo::Storage>, public TimerNotify {
		friend class utils::Singleton<Storage>;
	private:
		Storage();
		~Storage();

		KeyValueDb *keyvalue_db_;
		KeyValueDb *ledger_db_;
		KeyValueDb *account_db_;

		//tidb?rocksdb/leveldb from conf file
		std::string db_type;

		bool CloseDb();
		bool DescribeTable(const std::string &name, const std::string &sql_create_table);
		bool ManualDescribeTables();

		KeyValueDb *NewKeyValueDb(const DbConfigure &db_config);
	public:
		bool Initialize(const DbConfigure &db_config, bool bdropdb);
		bool Initialize_Tidb(const DbConfigure &db_config, bool bdropdb);
		bool Exit();
		std::string get_db_type();

		KeyValueDb *keyvalue_db();   //storage others
		KeyValueDb *account_db();   //storage account tree
		KeyValueDb *ledger_db();    //storage transaction and ledger

		//sync account db and ledger db
		utils::ReadWriteLock account_ledger_lock_;

		virtual void OnTimer(int64_t current_time) {};
		virtual void OnSlowTimer(int64_t current_time);
	};
}

#endif