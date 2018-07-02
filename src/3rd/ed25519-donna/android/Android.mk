
LOCAL_PATH:= $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../
SRC_PATH:= $(TOP_PATH)/

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_ED25519_FILES := \
 			$(SRC_PATH)/ed25519.c 

LOCAL_SRC_FILES := \
	$(LOCAL_SRC_ED25519_FILES) 
	

LOCAL_C_INCLUDES += \
	$(SRC_PATH)/ \
	$(TOP_PATH)/../asio/include \
	$(TOP_PATH)/../basic/include \
	$(TOP_PATH)/../../

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog 

LOCAL_CPPFLAGS := -DHAMMER_TIME=1 \
		  -DHASHNAMESPACE=__gnu_cxx \
		  -DHASH_NAMESPACE=__gnu_cxx \
		  -DDISABLE_DYNAMIC_CAST \
		  -DASIO_STANDALONE \
		  -DANDROID \
		  -DOS_ANDROID

LOCAL_CFLAGS := -fexpensive-optimizations -fexceptions -pthread -DHAVE_NEON=1 \
		-mfpu=neon -mfloat-abi=softfp -flax-vector-conversions -fPIC -D__STDC_CONSTANT_MACROS -Wno-sign-compare -Wno-switch -DOS_ANDROID

LOCAL_MODULE:= libed25519

include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)

