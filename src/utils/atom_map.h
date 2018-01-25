#ifndef TEMPLATE_ATOMIC_MAP_H
#define TEMPLATE_ATOMIC_MAP_H

#include <map>
#include <string>
#include <memory>
#include <exception>
#include "logger.h"

namespace bumo
{
	template<class KEY, class VALUE, class COMPARE = std::less<KEY>>
	class AtomMap
	{
	public:
		typedef std::shared_ptr<VALUE> pointer;

		enum actType
		{
			ADD = 0,
			MOD = 1,
			DEL = 2,
			REV = 3,
			MAX,
		};

		struct ActValue
		{
			pointer value_;
			actType type_;
			ActValue(actType type = MAX) :type_(type){}
			ActValue(const pointer& val, actType type = MAX) :value_(val), type_(type){}
		};

		typedef std::map<KEY, ActValue, COMPARE> mapKV;

	protected:
		mapKV  actionBuf_;
		mapKV  standby_;
		mapKV* data_;

	public:
		AtomMap()
		{
			data_ = &standby_; //avoid manual memory management
		}

		AtomMap(mapKV* data)
		{
			if (data)
				data_ = data;
			else
				data_ = &standby_; //avoid manual memory management
		}

	private:
		void SetValue(const KEY& key, const pointer& val)
		{
			actionBuf_[key] = ActValue(val, MOD);
		}

		bool GetValue(const KEY& key, pointer& val)
		{
			bool ret = false;
			auto itAct = actionBuf_.find(key);
			if (itAct != actionBuf_.end())
			{
				if (itAct->second.type_ == DEL)
					return false;

				val = itAct->second.value_;
				ret = true;
			}
			else
			{
				auto itData = data_->find(key);
				if (itData != data_->end())
				{
					if (itData->second.type_ == DEL)
						return false;

					//can't be assigned directly, because itData->second.value_ is smart pointer
					auto pv = std::make_shared<VALUE>(*(itData->second.value_));
					if (!pv)
						return false;

					actionBuf_[key] = ActValue(pv, MOD);
					val = pv;
					ret = true;
				}
				else
				{
					if (!GetFromDB(key, val))
						return false;

					actionBuf_[key] = ActValue(val, ADD);
					ret = true;
				}
			}
			return ret;
		}

	public:
		const mapKV& GetData()
		{
			return *data_;
		}

		mapKV& GetActionBuf()
		{
			return actionBuf_;
		}

		bool Set(const KEY& key, const pointer& val)
		{
			bool ret = true;

			try{ SetValue(key, val); }
			catch(std::exception& e)
			{ 
				LOG_ERROR("set exception, detail: %s", e.what());
				ret = false;
			}

			return ret;
		}

		bool Get(const KEY& key, pointer& val)
		{
			bool ret = true;

			try{ ret = GetValue(key, val); }
			catch(std::exception& e)
			{ 
				LOG_ERROR("get exception, detail: %s", e.what());
				ret = false;
			}
			return ret;
		}

		bool Del(const KEY& key)
		{
			bool ret = true;

			try{ actionBuf_[key] = ActValue(DEL); }
			catch(std::exception& e)
			{ 
				LOG_ERROR("delete exception, detail: %s", e.what());
				ret = false;
			}

			return ret;
		}

	private:
		bool CopyCommit()
		{
			mapKV copyBuf = *data_;
			try
			{
				for (auto act : actionBuf_)
					copyBuf[act.first] = act.second;
			}
			catch (std::exception& e)
			{
				LOG_ERROR("copy commit exception, detail: %s", e.what());
				actionBuf_.clear();
				return false;
			}

			data_->swap(copyBuf);

			//CAUTION: now the pointers in actionBuf_ and dataCopy_ are overlapped with data_,
			//so must be clear, otherwise the later modification to them will aslo directly act on data_.
			actionBuf_.clear(); 
			return true;
		}

	public:
		bool Commit()
		{
			return CopyCommit();
		}

		//call ClearChange to discard the modification if Commit failed
		void ClearChangeBuf()
		{
			actionBuf_.clear();
		}

		virtual bool GetFromDB(const KEY& key, pointer& val){ return false; }
		virtual void updateToDB(){}
	};

	template<class KEY, class VALUE, class COMPARE = std::less<KEY>>
	class AtomBaseMap
	{
	public:
		typedef std::map<KEY, VALUE, COMPARE> mapKV;

	protected:
		mapKV  actionBuf_;
		mapKV  standby_;
		mapKV* data_;

	public:
		AtomBaseMap()
		{
			data_ = &standby_; //avoid manual memory management
		}

		AtomBaseMap(mapKV* data)
		{
			if (data)
				data_ = data;
			else
				data_ = &standby_; //avoid manual memory management
		}

		void SetValue(const KEY& key, const VALUE& val)
		{
			actionBuf_[key] = val;
		}

		bool GetValue(const KEY& key, VALUE& val)
		{
			bool ret = false;
			auto itAct = actionBuf_.find(key);
			if (itAct != actionBuf_.end())
			{
				val = actionBuf_[key];
				ret = true;
			}
			else
			{
				auto itData = data_->find(key);
				if (itData != data_->end())
				{
					actionBuf_[key] = (*data_)[key];
					val = actionBuf_[key];
					ret = true;
				}
			}
			return ret;
		}

		const mapKV& GetData()
		{
			return *data_;
		}

		mapKV& GetActionBuf()
		{
			return actionBuf_;
		}

		void Commit()
		{
			for (auto act : actionBuf_)
				(*data_)[act.first] = act.second;

			actionBuf_.clear();
		}

		void ClearChangeBuf()
		{
			actionBuf_.clear();
		}
	};
}

#endif //TEMPLATE_ATOMIC_MAP_H
