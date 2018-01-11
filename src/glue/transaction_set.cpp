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

#include "transaction_set.h"

namespace bumo {
	TransactionSetFrm::TransactionSetFrm(const protocol::TransactionEnvSet &env) {
		raw_txs_ = env;
	}

	TransactionSetFrm::TransactionSetFrm() {
	}

	TransactionSetFrm::~TransactionSetFrm() {}
	int32_t TransactionSetFrm::Add(const TransactionFrm::pointer &tx) {
		if (raw_txs_.ByteSize() + tx->GetTransactionEnv().ByteSize() >= General::TXSET_LIMIT_SIZE) {
			LOG_ERROR("Txset byte size(%d) will be exceed than limit(%d), stop added current tx(size:%d)", 
				raw_txs_.ByteSize(), 
				General::TXSET_LIMIT_SIZE,
				tx->GetTransactionEnv().ByteSize());
			return 0;
		} 

		int64_t last_seq = 0;
		do {
			//find this cache
			std::map<std::string, int64_t>::iterator this_iter = topic_seqs_.find(tx->GetSourceAddress());
			if (this_iter != topic_seqs_.end()) {
				last_seq = this_iter->second;
				break;
			}

			//find global cache
			AccountFrm::pointer account;
			if (Environment::AccountFromDB(tx->GetSourceAddress(), account)) {
				last_seq = account->GetAccountNonce();
			}
		} while (false);

		if (tx->GetNonce() > last_seq + 1) {
			LOG_ERROR("The tx seq(" FMT_I64 ") is large than last seq(" FMT_I64 ") + 1", tx->GetNonce(), last_seq);
			return 0;
		}

		if (tx->GetNonce() <= last_seq) {
			LOG_ERROR("The tx seq(" FMT_I64 ") is less or equal of last seq(" FMT_I64 "), remove it", tx->GetNonce(), last_seq);
			return -1;
		}

		topic_seqs_[tx->GetSourceAddress()] = tx->GetNonce();
		*raw_txs_.add_txs() = tx->GetProtoTxEnv();
		return 1;
	}

	bool TransactionSetFrm::CheckValid() const{
		if (raw_txs_.ByteSize() >= General::TXSET_LIMIT_SIZE) {
			LOG_WARN("The txset size(%d) will be exceed the limit(%d), check invalid",
				raw_txs_.ByteSize(), General::TXSET_LIMIT_SIZE);
			return 0;
		}

		std::string last_address;
		int64_t last_seq = -1;
		for (int32_t i = 0; i < raw_txs_.txs_size(); i++) {
			const protocol::TransactionEnv &env = raw_txs_.txs(i);
			TransactionFrm tx(env);
			if (!tx.CheckValid(-1)) {
				LOG_ERROR("Check txset failed");
				return false;
			}

			const std::string &address = env.transaction().source_address();
			int64_t seq = env.transaction().nonce();
			if (last_seq < 0 || address > last_address) {
				last_seq = seq;
				last_address = address;
			} else {
				if ((address == last_address) && seq == last_seq + 1){
					last_seq = seq;
					continue;
				}
				else {
					Json::Value json_raw = Proto2Json(raw_txs_);
					LOG_ERROR("Check txset failed, as not order(%s)", json_raw.toFastString().c_str());
					return false;
				}
			}
		}

		return true;
	}

	std::string TransactionSetFrm::GetSerializeString() const {
		return raw_txs_.SerializeAsString();
	}

	int32_t TransactionSetFrm::Size() const {
		return raw_txs_.txs_size();
	}

	const protocol::TransactionEnvSet &TransactionSetFrm::GetRaw() const {
		return raw_txs_;
	}

	TopicKey::TopicKey() : sequence_(0) {}
	TopicKey::TopicKey(const std::string &topic, int64_t sequence) : topic_(topic), sequence_(sequence) {}
	TopicKey::~TopicKey() {}

	bool TopicKey::operator<(const TopicKey &key) const {
		if (topic_ < key.topic_) {
			return true;
		}
		else if (topic_ == key.topic_ && sequence_ < key.sequence_) {
			return true;
		}

		return false;
	}

	const std::string &TopicKey::GetTopic() const {
		return topic_;
	}

	const int64_t TopicKey::GetSeq() const {
		return sequence_;
	}
}