APP_PROJECT_PATH := $(call my-dir)/../
project-root-dir:=$(APP_PROJECT_PATH)
APP_BUILD_SCRIPT:=$(call my-dir)/Android.mk
APP_STL := gnustl_static #GNU STL  
APP_CPPFLAGS := -fexceptions -frtti #允许异常功能，及运行时类型识别  
APP_CPPFLAGS += -std=gnu++11  #允许使用c++11的函数等功能  
APP_CPPFLAGS +=-fpermissive  #此项有效时表示宽松的编译形式，比如没有用到的代码中有错误也可以通过
APP_ABI := armeabi-v7a
APP_PLATFORM := android-24 
APP_MODULES := bu
