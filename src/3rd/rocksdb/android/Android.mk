
LOCAL_PATH:= $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../
SRC_PATH:= $(TOP_PATH)/

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_ROCKSDB_FILES := \
		  $(SRC_PATH)/db/builder.cc                                                 \
		  $(SRC_PATH)/db/c.cc                                                       \
		  $(SRC_PATH)/db/column_family.cc                                           \
		  $(SRC_PATH)/db/compaction.cc                                              \
		  $(SRC_PATH)/db/compaction_job.cc                                          \
		  $(SRC_PATH)/db/compaction_picker.cc                                       \
		  $(SRC_PATH)/db/db_filesnapshot.cc                                         \
		  $(SRC_PATH)/db/dbformat.cc                                                \
		  $(SRC_PATH)/db/db_impl.cc                                                 \
		  $(SRC_PATH)/db/db_impl_debug.cc                                           \
		  $(SRC_PATH)/db/db_impl_readonly.cc                                        \
		  $(SRC_PATH)/db/db_iter.cc                                                 \
		  $(SRC_PATH)/db/file_indexer.cc                                            \
		  $(SRC_PATH)/db/filename.cc                                                \
		  $(SRC_PATH)/db/flush_job.cc                                               \
		  $(SRC_PATH)/db/flush_scheduler.cc                                         \
		  $(SRC_PATH)/db/forward_iterator.cc                                        \
		  $(SRC_PATH)/db/internal_stats.cc                                          \
		  $(SRC_PATH)/db/log_reader.cc                                              \
		  $(SRC_PATH)/db/log_writer.cc                                              \
		  $(SRC_PATH)/db/managed_iterator.cc                                        \
		  $(SRC_PATH)/db/memtable_allocator.cc                                      \
		  $(SRC_PATH)/db/memtable.cc                                                \
		  $(SRC_PATH)/db/memtable_list.cc                                           \
		  $(SRC_PATH)/db/merge_helper.cc                                            \
		  $(SRC_PATH)/db/merge_operator.cc                                          \
		  $(SRC_PATH)/db/repair.cc                                                  \
		  $(SRC_PATH)/db/slice.cc                                                  \
		  $(SRC_PATH)/db/table_cache.cc                                             \
		  $(SRC_PATH)/db/table_properties_collector.cc                              \
		  $(SRC_PATH)/db/transaction_log_impl.cc                                    \
		  $(SRC_PATH)/db/version_builder.cc                                         \
		  $(SRC_PATH)/db/version_edit.cc                                            \
		  $(SRC_PATH)/db/version_set.cc                                             \
		  $(SRC_PATH)/db/wal_manager.cc                                             \
		  $(SRC_PATH)/db/write_batch.cc                                             \
		  $(SRC_PATH)/db/write_batch_base.cc                                        \
		  $(SRC_PATH)/db/write_controller.cc                                        \
		  $(SRC_PATH)/db/write_thread.cc                                            \
		  $(SRC_PATH)/port/stack_trace.cc                                           \
		  $(SRC_PATH)/port/port_posix.cc                                            \
		  $(SRC_PATH)/table/adaptive_table_factory.cc                               \
		  $(SRC_PATH)/table/block_based_filter_block.cc                             \
		  $(SRC_PATH)/table/block_based_table_builder.cc                            \
		  $(SRC_PATH)/table/block_based_table_factory.cc                            \
		  $(SRC_PATH)/table/block_based_table_reader.cc                             \
		  $(SRC_PATH)/table/block_builder.cc                                        \
		  $(SRC_PATH)/table/block.cc                                                \
		  $(SRC_PATH)/table/block_hash_index.cc                                     \
		  $(SRC_PATH)/table/block_prefix_index.cc                                   \
		  $(SRC_PATH)/table/bloom_block.cc                                          \
		  $(SRC_PATH)/table/cuckoo_table_builder.cc                                 \
		  $(SRC_PATH)/table/cuckoo_table_factory.cc                                 \
		  $(SRC_PATH)/table/cuckoo_table_reader.cc                                  \
		  $(SRC_PATH)/table/flush_block_policy.cc                                   \
		  $(SRC_PATH)/table/format.cc                                               \
		  $(SRC_PATH)/table/full_filter_block.cc                                    \
		  $(SRC_PATH)/table/get_context.cc                                          \
		  $(SRC_PATH)/table/iterator.cc                                             \
		  $(SRC_PATH)/table/merger.cc                                               \
		  $(SRC_PATH)/table/meta_blocks.cc                                          \
		  $(SRC_PATH)/table/plain_table_builder.cc                                  \
		  $(SRC_PATH)/table/plain_table_factory.cc                                  \
		  $(SRC_PATH)/table/plain_table_index.cc                                    \
		  $(SRC_PATH)/table/plain_table_key_coding.cc                               \
		  $(SRC_PATH)/table/plain_table_reader.cc                                   \
		  $(SRC_PATH)/table/table_properties.cc                                     \
		  $(SRC_PATH)/table/two_level_iterator.cc                                   \
		  $(SRC_PATH)/util/arena.cc                                                 \
		  $(SRC_PATH)/util/auto_roll_logger.cc                                      \
		  $(SRC_PATH)/util/bloom.cc                                                 \                                       \
		  $(SRC_PATH)/util/cache.cc                                                 \
		  $(SRC_PATH)/util/coding.cc                                                \
		  $(SRC_PATH)/util/comparator.cc                                            \
		  $(SRC_PATH)/util/crc32c.cc                                                \
		  $(SRC_PATH)/util/db_info_dumper.cc                                        \
		  $(SRC_PATH)/util/dynamic_bloom.cc                                         \
		  $(SRC_PATH)/util/env.cc                                                   \
		  $(SRC_PATH)/util/env_hdfs.cc                                              \
		  $(SRC_PATH)/util/env_posix.cc                                             \
		  $(SRC_PATH)/util/file_util.cc                                             \
		  $(SRC_PATH)/util/filter_policy.cc                                         \
		  $(SRC_PATH)/util/hash.cc                                                  \
		  $(SRC_PATH)/util/hash_cuckoo_rep.cc                                       \
		  $(SRC_PATH)/util/hash_linklist_rep.cc                                     \
		  $(SRC_PATH)/util/hash_skiplist_rep.cc                                     \
		  $(SRC_PATH)/util/histogram.cc                                             \
		  $(SRC_PATH)/util/instrumented_mutex.cc                                    \
		  $(SRC_PATH)/util/iostats_context.cc                                       \
		  $(SRC_PATH)/utilities/backupable/backupable_db.cc                         \
		  $(SRC_PATH)/utilities/convenience/convenience.cc                          \
		  $(SRC_PATH)/utilities/checkpoint/checkpoint.cc                            \
		  $(SRC_PATH)/utilities/compacted_db/compacted_db_impl.cc                   \
		  $(SRC_PATH)/utilities/document/document_db.cc                             \
		  $(SRC_PATH)/utilities/document/json_document_builder.cc                   \
		  $(SRC_PATH)/utilities/document/json_document.cc                           \
		  $(SRC_PATH)/utilities/flashcache/flashcache.cc                            \
		  $(SRC_PATH)/utilities/geodb/geodb_impl.cc                                 \
		  $(SRC_PATH)/utilities/leveldb_options/leveldb_options.cc                  \
		  $(SRC_PATH)/utilities/merge_operators/put.cc                              \
		  $(SRC_PATH)/utilities/merge_operators/string_append/stringappend2.cc      \
		  $(SRC_PATH)/utilities/merge_operators/string_append/stringappend.cc       \
		  $(SRC_PATH)/utilities/merge_operators/uint64add.cc                        \
		  $(SRC_PATH)/utilities/redis/redis_lists.cc                                \
		  $(SRC_PATH)/utilities/spatialdb/spatial_db.cc                             \
		  $(SRC_PATH)/utilities/ttl/db_ttl_impl.cc                                  \
		  $(SRC_PATH)/utilities/write_batch_with_index/write_batch_with_index.cc    \
		  $(SRC_PATH)/util/event_logger.cc                                          \
		  $(SRC_PATH)/util/ldb_cmd.cc                                               \
		  $(SRC_PATH)/util/ldb_tool.cc                                              \
		  $(SRC_PATH)/util/log_buffer.cc                                            \
		  $(SRC_PATH)/util/logging.cc                                               \
		  $(SRC_PATH)/util/memenv.cc                                                \
		  $(SRC_PATH)/util/murmurhash.cc                                            \
		  $(SRC_PATH)/util/mutable_cf_options.cc                                    \
		  $(SRC_PATH)/util/options_builder.cc                                       \
		  $(SRC_PATH)/util/options.cc                                               \
		  $(SRC_PATH)/util/options_helper.cc                                        \
		  $(SRC_PATH)/util/perf_context.cc                                          \
		  $(SRC_PATH)/util/rate_limiter.cc                                          \
		  $(SRC_PATH)/util/skiplistrep.cc                                           \
		  $(SRC_PATH)/util/slice.cc                                                 \
		  $(SRC_PATH)/util/sst_dump_tool.cc                                         \
		  $(SRC_PATH)/util/statistics.cc                                            \
		  $(SRC_PATH)/util/status.cc                                                \
		  $(SRC_PATH)/util/string_util.cc                                           \
		  $(SRC_PATH)/util/sync_point.cc                                            \
		  $(SRC_PATH)/util/thread_local.cc                                          \
		  $(SRC_PATH)/util/thread_status_impl.cc                                    \
		  $(SRC_PATH)/util/thread_status_updater.cc                                 \
		  $(SRC_PATH)/util/thread_status_updater_debug.cc                           \
		  $(SRC_PATH)/util/thread_status_util.cc                                    \
		  $(SRC_PATH)/util/thread_status_util_debug.cc                              \
		  $(SRC_PATH)/util/vectorrep.cc                                             \
		  $(SRC_PATH)/util/xfunc.cc                                                 \
		  $(SRC_PATH)/util/xxhash.cc                                                

LOCAL_SRC_FILES := \
	$(LOCAL_SRC_ROCKSDB_FILES) 
	
APP_STL:=c++_shared
 
LOCAL_C_INCLUDES += \
	$(SRC_PATH)/ \
	$(TOP_PATH)/include

#LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog 

LOCAL_CPPFLAGS := -DHAMMER_TIME=1 \
		  -DHASHNAMESPACE=__gnu_cxx \
		  -DHASH_NAMESPACE=__gnu_cxx \
		  -fno-builtin-memcmp \
		  -D_REENTRANT \
		  -DOS_ANDROID \
		  -DLEVELDB_PLATFORM_POSIX \
		  -DROCKSDB_LITE \
		  -DROCKSDB_PLATFORM_POSIX

LOCAL_CFLAGS := -fexpensive-optimizations -fexceptions -pthread -DHAVE_NEON=1 \
		-mfpu=neon -std=c++11 -O3 -mfloat-abi=softfp -flax-vector-conversions -fPIC -D__STDC_CONSTANT_MACROS -Wno-sign-compare -Wno-switch 

LOCAL_MODULE:= librocksdb

include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)

