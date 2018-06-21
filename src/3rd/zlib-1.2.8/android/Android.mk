
LOCAL_PATH:= $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../
SRC_PATH:= $(TOP_PATH)/

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
 			$(SRC_PATH)/compress.c \
 			$(SRC_PATH)/deflate.c \
			$(SRC_PATH)/infback.c \
			$(SRC_PATH)/crc32.c \
			$(SRC_PATH)/inftrees.c \
			$(SRC_PATH)/adler32.c \
			$(SRC_PATH)/gzclose.c \
			$(SRC_PATH)/gzlib.c \
			$(SRC_PATH)/gzread.c \
			$(SRC_PATH)/gzwrite.c \
			$(SRC_PATH)/inflate.c \
			$(SRC_PATH)/inffast.c \
			$(SRC_PATH)/trees.c \
			$(SRC_PATH)/uncompr.c \
			$(SRC_PATH)/zutil.c	

LOCAL_C_INCLUDES += \
	$(SRC_PATH)/ \
	$(TOP_PATH)/include

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog 

LOCAL_CPPFLAGS := -DHAMMER_TIME=1 \
		  -DHASHNAMESPACE=__gnu_cxx \
		  -DHASH_NAMESPACE=__gnu_cxx \
		  -DDISABLE_DYNAMIC_CAST \
		  -D_REENTRANT \
		  -DANDROID

LOCAL_CFLAGS := -fexpensive-optimizations -fexceptions -pthread -DHAVE_NEON=1 \
		-mfpu=neon -mfloat-abi=softfp -flax-vector-conversions -fPIC -D__STDC_CONSTANT_MACROS -Wno-sign-compare -Wno-switch 

LOCAL_MODULE:= libzlib

include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)
