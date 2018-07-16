
#
# Copyright (C) The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4.1)

# configure import libs
set(distribution_DIR ${CMAKE_SOURCE_DIR}/../../../../distribution)

# shared lib will also be tucked into APK and sent to target
# refer to app/build.gradle, jniLibs section for that purpose.
# ${ANDROID_ABI} is handy for our purpose here. Probably this ${ANDROID_ABI} is
# the most valuable thing of this sample, the rest are pretty much normal cmake

add_library(lib_v8_libbase SHARED IMPORTED)
add_library(lib_v8_libplatform SHARED IMPORTED)
add_library(lib_v8 SHARED IMPORTED)
add_library(lib_bu SHARED IMPORTED)
add_library(lib_icui18n SHARED IMPORTED)
add_library(lib_icuuc SHARED IMPORTED)

set_target_properties(lib_v8_libbase PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/bu/lib/${ANDROID_ABI}/libv8_libbase.so
    )

set_target_properties(lib_v8_libplatform PROPERTIES IMPORTED_LOCATION
     ${distribution_DIR}/bu/lib/${ANDROID_ABI}/libv8_libplatform.so
     )



set_target_properties(lib_v8 PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/bu/lib/${ANDROID_ABI}/libv8.so
    )

set_target_properties(lib_bu PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/bu/lib/${ANDROID_ABI}/libbu.so
    )

set_target_properties(lib_icui18n PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/bu/lib/${ANDROID_ABI}/libicui18n.so
)

set_target_properties(lib_icuuc PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/bu/lib/${ANDROID_ABI}/libicuuc.so
    )


# build application's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_library(hello-libs SHARED
            hello-libs.cpp)

target_include_directories(hello-libs PRIVATE
                           ${distribution_DIR}/bu/include)

target_link_libraries(hello-libs
                      android
                      lib_bu
                      lib_v8
                      lib_v8_libbase
                      lib_v8_libplatform
                      lib_icui18n
                      lib_icuuc
                      log)
