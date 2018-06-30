APP_PROJECT_PATH := $(call my-dir)/../
project-root-dir:=$(APP_PROJECT_PATH)
APP_BUILD_SCRIPT:=$(call my-dir)/Android.mk
APP_STL := gnustl_static
APP_ABI := armeabi-v7a 
APP_PLATFORM := android-21 
APP_MODULES := libscrypt
APP_CPPFLAGS += -frtti