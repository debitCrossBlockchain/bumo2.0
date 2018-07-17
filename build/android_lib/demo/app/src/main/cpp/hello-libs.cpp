/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <cstring>
#include <jni.h>
#include <cinttypes>
#include <android/log.h>
#include <bu.h>
#include <string>

void android_log(const char* file_name, const char * format, ...)
{
    va_list apptr;
    va_start(apptr, format);
    __android_log_vprint(ANDROID_LOG_DEBUG, file_name, format, apptr);
    va_end(apptr);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_hellolibs_MainActivity_buInit(JNIEnv *env, jobject thiz) {
    android_log("TraceLog", "bu init in");
    int result = Init("/sdcard/bumo");
    char temp[1000] = {0};
    sprintf(temp, "Init buchain: %d", result);
    android_log("TraceLog", "bu init out");
    return env->NewStringUTF(temp);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_hellolibs_MainActivity_buUnInit(JNIEnv *env, jobject thiz) {
    android_log("TraceLog", "bu uninit in");
    int result = UnInit();
    android_log("TraceLog", "bu uninit in-1");
    char temp[1000] = {0};
    sprintf(temp, "UnInit buchain: %d", result);
    android_log("TraceLog", "bu uninit out");
    return env->NewStringUTF(temp);
}

