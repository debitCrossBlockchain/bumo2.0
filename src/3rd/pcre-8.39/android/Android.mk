#
#  Android makefile for libpcre
#
#  This makefile generates libpcre.a, pcregrep and pcre.h ONLY.
#  It should be amended to build libpcreposix.a, libpcrecpp.a
#  and tests.


LOCAL_PATH := $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../

###
### Build libpcre.a and pcre.h
###

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libpcre
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := samples

$(info $(shell (cp $(TOP_PATH)/pcre_chartables.c.dist $(TOP_PATH)/pcre_chartables.c)))
$(info $(shell (cp $(TOP_PATH)/config.h.generic $(TOP_PATH)/config.h)))

intermediates := $(call local-intermediates-dir)

LOCAL_SRC_FILES :=  \
  $(TOP_PATH)/pcre_compile.c \
  $(TOP_PATH)/pcre_config.c \
  $(TOP_PATH)/pcre_dfa_exec.c \
  $(TOP_PATH)/pcre_exec.c \
  $(TOP_PATH)/pcre_fullinfo.c \
  $(TOP_PATH)/pcre_get.c \
  $(TOP_PATH)/pcre_globals.c \
  $(TOP_PATH)/pcre_maketables.c \
  $(TOP_PATH)/pcre_newline.c \
  $(TOP_PATH)/pcre_ord2utf8.c \
  $(TOP_PATH)/pcre_refcount.c \
  $(TOP_PATH)/pcre_study.c \
  $(TOP_PATH)/pcre_tables.c \
  $(TOP_PATH)/pcre_ucd.c \
  $(TOP_PATH)/pcre_valid_utf8.c \
  $(TOP_PATH)/pcre_version.c \
  $(TOP_PATH)/pcre_xclass.c \
  $(TOP_PATH)/pcre_chartables.c

LOCAL_COPY_HEADERS := pcre.h

LOCAL_CFLAGS += -O3 -I. -DHAVE_CONFIG_H

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)