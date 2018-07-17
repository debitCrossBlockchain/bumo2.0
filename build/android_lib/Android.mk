
LOCAL_PATH:= $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../../
SRC_PATH:= $(TOP_PATH)/

#static_libs
include $(CLEAR_VARS)
LOCAL_MODULE:=libcurl
LOCAL_SRC_FILES:=$(SRC_PATH)/src/3rd/curl/lib/.libs/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libssl
LOCAL_SRC_FILES:=$(SRC_PATH)/src/3rd/openssl/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libcrypto
LOCAL_SRC_FILES:=$(SRC_PATH)/src/3rd/openssl/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libv8_libbase
LOCAL_SRC_FILES:=$(SRC_PATH)/src/3rd/v8_target/android/libv8_libbase.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libv8
LOCAL_SRC_FILES:=$(SRC_PATH)/src/3rd/v8_target/android/libv8.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libv8_libplatform
LOCAL_SRC_FILES:=$(SRC_PATH)/src/3rd/v8_target/android/libv8_libplatform.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libicui18n
LOCAL_SRC_FILES:=$(SRC_PATH)/src/3rd/v8_target/android/libicui18n.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libicuuc
LOCAL_SRC_FILES:=$(SRC_PATH)/src/3rd/v8_target/android/libicuuc.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)


LOCAL_SRC_MAIN_FILES := \
			$(LOCAL_PATH)/src/bu.cpp\
			$(SRC_PATH)/src/main/configure.cpp

LOCAL_SRC_API_FILES := \
 			$(SRC_PATH)/src/api/console.cpp\
			$(SRC_PATH)/src/api/web_server.cpp\
			$(SRC_PATH)/src/api/web_server_command.cpp\
			$(SRC_PATH)/src/api/web_server_helper.cpp\
			$(SRC_PATH)/src/api/web_server_query.cpp\
			$(SRC_PATH)/src/api/web_server_update.cpp\
			$(SRC_PATH)/src/api/websocket_server.cpp

LOCAL_SRC_GLUE_FILES := \
 			$(SRC_PATH)/src/glue/glue_manager.cpp\
			$(SRC_PATH)/src/glue/ledger_upgrade.cpp\
			$(SRC_PATH)/src/glue/transaction_queue.cpp\
			$(SRC_PATH)/src/glue/transaction_set.cpp

LOCAL_SRC_LEDGER_FILES := \
 			$(SRC_PATH)/src/ledger/account.cpp\
			$(SRC_PATH)/src/ledger/contract_manager.cpp\
			$(SRC_PATH)/src/ledger/environment.cpp\
			$(SRC_PATH)/src/ledger/fee_calculate.cpp\
			$(SRC_PATH)/src/ledger/kv_trie.cpp\
			$(SRC_PATH)/src/ledger/ledger_frm.cpp\
			$(SRC_PATH)/src/ledger/ledger_manager.cpp\
			$(SRC_PATH)/src/ledger/ledgercontext_manager.cpp\
			$(SRC_PATH)/src/ledger/operation_frm.cpp\
			$(SRC_PATH)/src/ledger/transaction_frm.cpp\
			$(SRC_PATH)/src/ledger/trie.cpp

LOCAL_SRC_MONITOR_FILES := \
 			$(SRC_PATH)/src/monitor/monitor.cpp\
			$(SRC_PATH)/src/monitor/monitor_manager.cpp\
			$(SRC_PATH)/src/monitor/system_manager.cpp

LOCAL_SRC_OVERLAY_FILES := \
 			$(SRC_PATH)/src/overlay/broadcast.cpp\
			$(SRC_PATH)/src/overlay/peer.cpp\
			$(SRC_PATH)/src/overlay/peer_manager.cpp\
			$(SRC_PATH)/src/overlay/peer_network.cpp


LOCAL_SRC_PROTO_FILES := \
 			$(SRC_PATH)/src/proto/cpp/chain.pb.cc\
			$(SRC_PATH)/src/proto/cpp/common.pb.cc\
			$(SRC_PATH)/src/proto/cpp/consensus.pb.cc\
			$(SRC_PATH)/src/proto/cpp/merkeltrie.pb.cc\
			$(SRC_PATH)/src/proto/cpp/monitor.pb.cc\
			$(SRC_PATH)/src/proto/cpp/overlay.pb.cc

LOCAL_SRC_CONSENSUS_FILES := \
 			$(SRC_PATH)/src/consensus/bft.cpp\
			$(SRC_PATH)/src/consensus/bft_instance.cpp\
			$(SRC_PATH)/src/consensus/consensus.cpp\
			$(SRC_PATH)/src/consensus/consensus_manager.cpp\
			$(SRC_PATH)/src/consensus/consensus_msg.cpp	
			
			
LOCAL_SRC_COMMON_FILES := \
 			$(SRC_PATH)/src/common/argument.cpp\
			$(SRC_PATH)/src/common/configure_base.cpp\
			$(SRC_PATH)/src/common/daemon.cpp\
			$(SRC_PATH)/src/common/data_secret_key.cpp\
			$(SRC_PATH)/src/common/general.cpp\
			$(SRC_PATH)/src/common/key_store.cpp\
			$(SRC_PATH)/src/common/network.cpp\
			$(SRC_PATH)/src/common/pb2json.cpp\
			$(SRC_PATH)/src/common/private_key.cpp\
			$(SRC_PATH)/src/common/storage.cpp

LOCAL_SRC_UTILS_FILES := \
 			$(SRC_PATH)/src/utils/file.cpp\
			$(SRC_PATH)/src/utils/logger.cpp\
			$(SRC_PATH)/src/utils/net.cpp\
			$(SRC_PATH)/src/utils/thread.cpp\
			$(SRC_PATH)/src/utils/timestamp.cpp\
			$(SRC_PATH)/src/utils/utils.cpp\
			$(SRC_PATH)/src/utils/crypto.cpp\
			$(SRC_PATH)/src/utils/timer.cpp\
			$(SRC_PATH)/src/utils/system.cpp\
			$(SRC_PATH)/src/utils/exprparser.cpp\
			$(SRC_PATH)/src/utils/sm3.cpp\
			$(SRC_PATH)/src/utils/ecc_sm2.cpp\
			$(SRC_PATH)/src/utils/random.cpp\
			$(SRC_PATH)/src/utils/stdlibs.cpp\
			$(SRC_PATH)/src/utils/android_ifaddrs/ifaddrs.cc

			
LOCAL_SRC_FILES := \
	$(LOCAL_SRC_MAIN_FILES)\
	$(LOCAL_SRC_API_FILES)\
	$(LOCAL_SRC_GLUE_FILES)\
	$(LOCAL_SRC_LEDGER_FILES)\
	$(LOCAL_SRC_MONITOR_FILES)\
	$(LOCAL_SRC_OVERLAY_FILES)\
	$(LOCAL_SRC_PROTO_FILES)\
	$(LOCAL_SRC_CONSENSUS_FILES)\
	$(LOCAL_SRC_COMMON_FILES)\
	$(LOCAL_SRC_UTILS_FILES)
	

LOCAL_C_INCLUDES += \
	$(SRC_PATH)/src/ \
	$(SRC_PATH)/src/main/ \
	$(SRC_PATH)/src/api/ \
	$(SRC_PATH)/src/glue/ \
	$(SRC_PATH)/src/ledger/ \
	$(SRC_PATH)/src/monitor/ \
	$(SRC_PATH)/src/overlay/ \
	$(SRC_PATH)/src/proto/cpp/ \
	$(SRC_PATH)/src/common/ \
	$(SRC_PATH)/src/consensus/ \
	$(SRC_PATH)/src/utils/ \
	$(SRC_PATH)/src/utils/android_ifaddrs/ \
    $(SRC_PATH)/src/3rd/basic/include/ \
    $(SRC_PATH)/src/3rd/protobuf/src/ \
    $(SRC_PATH)/src/3rd/basic/include/ \
    $(SRC_PATH)/src/3rd/jsoncpp/include/ \
    $(SRC_PATH)/src/3rd/sqlite/ \
    $(SRC_PATH)/src/3rd/rocksdb/include/ \
    $(SRC_PATH)/src/3rd/websocketpp/ \
    $(SRC_PATH)/src/3rd/bzip2-1.0.6/ \
    $(SRC_PATH)/src/3rd/zlib-1.2.8/ \
    $(SRC_PATH)/src/3rd/asio/include/ \
    $(SRC_PATH)/src/3rd/http/ \
    $(SRC_PATH)/src/3rd/libscrypt/ \
    $(SRC_PATH)/src/3rd/basic/include/v8/ \
    $(LOCAL_PATH)/src/ \
	$(LOCAL_PATH)/interface/ \

LOCAL_STATIC_LIBRARIES:=libcurl libssl libcrypto libjson libbz2 libzlib libscrypt libprotobuf libed25519 libhttp libleveldb 
LOCAL_SHARED_LIBRARIES:=libv8 libv8_libbase libv8_libplatform  libicui18n  libicuuc

LOCAL_LDLIBS += -llog 
LOCAL_CPPFLAGS := -D_REENTRANT \
		  -DOS_ANDROID \
		  -DASIO_STANDALONE \
		  -DASIO_HAS_STD_ADDRESSOF \
		  -DASIO_HAS_STD_ATOMIC \
		  -DASIO_HAS_STD_TYPE_TRAITS \
		  -DASIO_HAS_STD_SHARED_PTR \
		  -DASIO_HAS_STD_ARRAY \
		  -DASIO_HAS_CSTDINT \
		  -D_WEBSOCKETPP_CPP11_STL_





LOCAL_MODULE:= libbu

include $(BUILD_SHARED_LIBRARY)
include $(BUMO_SRC_PATH)3rd/bzip2-1.0.6/android/Android.mk
include $(BUMO_SRC_PATH)3rd/ed25519-donna/android/Android.mk
include $(BUMO_SRC_PATH)3rd/http/android/Android.mk
include $(BUMO_SRC_PATH)3rd/jsoncpp/android/Android.mk
include $(BUMO_SRC_PATH)3rd/libscrypt/android/Android.mk
include $(BUMO_SRC_PATH)3rd/protobuf/android/Android.mk
include $(BUMO_SRC_PATH)3rd/leveldb/android/Android.mk
include $(BUMO_SRC_PATH)3rd/zlib-1.2.8/android/Android.mk



