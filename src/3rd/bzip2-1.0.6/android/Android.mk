
LOCAL_PATH:= $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../
SRC_PATH:= $(TOP_PATH)/

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_BZ2_FILES := \
 			$(SRC_PATH)/blocksort.c \
			$(SRC_PATH)/huffman.c \
			$(SRC_PATH)/crctable.c \
			$(SRC_PATH)/randtable.c \
			$(SRC_PATH)/compress.c \
			$(SRC_PATH)/decompress.c \
			$(SRC_PATH)/bzip2recover.c \
			$(SRC_PATH)/bzlib.c

LOCAL_SRC_FILES := \
	$(LOCAL_SRC_BZ2_FILES) 
	

LOCAL_C_INCLUDES += \
	$(SRC_PATH)/ \
	$(TOP_PATH)/include

#LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog 

LOCAL_CPPFLAGS := -DHAMMER_TIME=1 \
		  -DHASHNAMESPACE=__gnu_cxx \
		  -DHASH_NAMESPACE=__gnu_cxx \
		  -DDISABLE_DYNAMIC_CAST \
		  -D_REENTRANT \
		  -DANDROID

LOCAL_CFLAGS := -fexpensive-optimizations -fexceptions -pthread -DHAVE_NEON=1 \
		-mfpu=neon -mfloat-abi=softfp -flax-vector-conversions -fPIC -D__STDC_CONSTANT_MACROS -Wno-sign-compare -Wno-switch 

LOCAL_MODULE:= libbz2

include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)

