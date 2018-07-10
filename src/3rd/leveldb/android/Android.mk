# Copyright (c) 2011 The LevelDB Authors. All rights reserved.                 # Use of this source code is governed by a BSD-style license that can be       # found in the LICENSE file. See the AUTHORS file for names of contributors.   

# To build for Android, add the Android NDK to your path and type 'ndk-build'.

LOCAL_PATH := $(call my-dir)
LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

include $(CLEAR_VARS)

include $(BUMO_SRC_PATH)3rd/leveldb/common.mk

LOCAL_MODULE := libleveldb
LOCAL_C_INCLUDES :=  $(BUMO_SRC_PATH)3rd/leveldb/include/ \
                     $(BUMO_SRC_PATH)3rd/leveldb/
LOCAL_CPP_EXTENSION := .cc
LOCAL_CFLAGS := -DLEVELDB_PLATFORM_ANDROID -std=gnu++0x
LOCAL_SRC_FILES := $(SOURCES:%.cc=../%.cc) $(BUMO_SRC_PATH)3rd/leveldb/port/port_android.cc

include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)
