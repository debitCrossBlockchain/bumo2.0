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
		bool   isRevertCommit_;
        mapKV  actionBuf_;
        mapKV  revertBuf_;
		mapKV  standby_;
        mapKV* data_;

	public:
		AtomMap(bool revertCommit = false) : isRevertCommit_(revertCommit)
		{
			data_ = &standby_; //avoid manual memory management
		}

		AtomMap(mapKV* data, bool revertCommit = false) : isRevertCommit_(revertCommit)
		{
			if (data)
				data_ = data;
			else
				data_ = &standby_; //avoid manual memory management
		}

	private:
		void DistinguishSetValue(const KEY& key, const pointer& val)
		{
			if (data_->find(key) == data_->end())
				actionBuf_[key] = ActValue(val, ADD);
			else
				actionBuf_[key] = ActValue(val, MOD);
		}

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
				if (itAct->second.type_ != DEL)
				{
					val = itAct->second.value_;
					ret = true;
				}
				//else ret = false;
			}
			else
			{
				auto itData = data_->find(key);
				if (itData != data_->end())
				{
					if (itData->second.type_ != DEL)
					{
						//can't be assigned directly, because itData->second.value_ is smart pointer
						auto pv = std::make_shared<VALUE>(*(itData->second.value_));
						if (pv)
						{
							actionBuf_[key] = ActValue(pv, MOD);
							val = pv;
							ret = true;
						}
						//else ret = false;
					}
					//else ret = false;
				}
				else
				{
					if (GetFromDB(key, val))
					{
						actionBuf_[key] = ActValue(val, ADD);
						ret = true;
					}
					//else ret = false;
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

			try
			{
				if (isRevertCommit_)
					DistinguishSetValue(key, val);
				else
					SetValue(key, val);
			}
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
        bool RevertCommit()
        {
            try
            {
				for (auto act : actionBuf_)
				{
					if(act.second.type_ == ADD)
						revertBuf_[act.first] = ActValue(REV); //if type_ == REV when UnCommit, data_.erase(key)
					else
						revertBuf_[act.first] = (*data_)[act.first];

					(*data_)[act.first] = act.second; //include type_ == DEL(UpdateToDB will use DEL and key)
				}
            }
            catch(std::exception& e)
            { 
                LOG_ERROR("commit exception, detail: %s", e.what());

				UnRevertCommit();
				actionBuf_.clear();
				revertBuf_.clear();

                return false;
            }

			//CAUTION: now the pointers in actionBuf_ and revertBuf_ are overlapped with data_,
			//so must be clear, otherwise the later modification to them will aslo directly act on data_.
			actionBuf_.clear();
			revertBuf_.clear();
            return true;
        }

		bool UnRevertCommit()
		{
            bool ret = true;

			try
			{
				for (auto rev : revertBuf_)
				{
					if (rev.second.type_ == REV)
						data_->erase(rev.first);
					else
						(*data_)[rev.first] = rev.second;
				}
			}
            catch(std::exception& e)
            { 
                LOG_ERROR("uncommit exception, detail: %s", e.what());
                ret = false;
            }

            return ret;
		}

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
			bool ret = false;

			if (isRevertCommit_)
				ret = RevertCommit();
			else
				ret = CopyCommit();

			return ret;
		}

		//call ClearChange to discard the modification if Commit failed
		void ClearChangeBuf()
		{
			actionBuf_.clear();
			revertBuf_.clear();
		}

        virtual bool GetFromDB(const KEY& key, pointer& val){ return false; }
		virtual void updateToDB(){}
	};
}

#endif //TEMPLATE_ATOMIC_MAP_H

