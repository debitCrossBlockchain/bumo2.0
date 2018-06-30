
LOCAL_PATH:= $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../../
SRC_PATH:= $(TOP_PATH)/


#static_libs
include $(CLEAR_VARS)
LOCAL_MODULE:=libcurl
LOCAL_SRC_FILES:=/bumo_3rd/curl/lib/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libcrypto
LOCAL_SRC_FILES:=/bumo_3rd/openssl/lib/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:=libssl
LOCAL_SRC_FILES:=/bumo_3rd/openssl/lib/libssl.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional


LOCAL_SRC_MAIN_FILES := \
			$(SRC_PATH)/src/main/main.cpp\
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
			$(SRC_PATH)/src/proto/cpp/helloworld.grpc.pb.cc\
			$(SRC_PATH)/src/proto/cpp/helloworld.pb.cc\
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
			$(SRC_PATH)/src/utils/sqlparser.cpp\
			$(SRC_PATH)/src/utils/exprparser.cpp\
			$(SRC_PATH)/src/utils/sm3.cpp\
			$(SRC_PATH)/src/utils/ecc_sm2.cpp\
			$(SRC_PATH)/src/utils/random.cpp\
			$(SRC_PATH)/src/utils/stdlibs.cpp

			
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
    $(SRC_PATH)/src/3rd/basic/include/ \
    $(SRC_PATH)/src/3rd/protobuf/src/ \
    $(SRC_PATH)/src/3rd/basic/include/ \
    $(SRC_PATH)/src/3rd/jsoncpp/include/ \
    $(SRC_PATH)/src/3rd/sqlite/ \
    $(SRC_PATH)/src/3rd/rocksdb/include/ \
    $(SRC_PATH)/src/3rd/pcre-8.39/ \
    $(SRC_PATH)/src/3rd/websocketpp/ \
    $(SRC_PATH)/src/3rd/bzip2-1.0.6/ \
    $(SRC_PATH)/src/3rd/zlib-1.2.8/ \
    $(SRC_PATH)/src/3rd/asio/include/ \
    $(SRC_PATH)/src/3rd/http/ \
    $(SRC_PATH)/src/3rd/libscrypt/ \
    $(SRC_PATH)/src/3rd/basic/include/v8/ \

LOCAL_STATIC_LIBRARIES:=libcurl libcrypto libssl

LOCAL_LDLIBS += -llog -landroid

LOCAL_CPPFLAGS := -DHAMMER_TIME=1 \
		  -DHASHNAMESPACE=__gnu_cxx \
		  -D_REENTRANT \
		  -DOS_ANDROID \
		  -DASIO_STANDALONE \
		  -DASIO_HAS_STD_ADDRESSOF \
		  -DASIO_HAS_STD_ATOMIC \
		  -DASIO_HAS_STD_TYPE_TRAITS \
		  -DASIO_HAS_STD_SHARED_PTR \
		  -DASIO_HAS_STD_ARRAY \
		  -DASIO_HAS_CSTDINT \
		  -D_WEBSOCKETPP_CPP11_STL_


LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE


LOCAL_CFLAGS := -fexpensive-optimizations -fexceptions -pthread -DHAVE_NEON=1 -mfpu=neon -mfloat-abi=softfp -flax-vector-conversions \
 -fPIC -D__STDC_CONSTANT_MACROS -Wno-sign-compare -Wno-switch -std=c++11 -std=c++0x

LOCAL_MODULE:= bumo

include $(BUILD_EXECUTABLE)

