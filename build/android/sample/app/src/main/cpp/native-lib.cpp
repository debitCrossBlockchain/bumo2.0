#include <jni.h>
#include <string>
#include "include/bumo.h"

extern "C" JNIEXPORT jstring

JNICALL
Java_com_example_root_bumodemo_bumo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


extern "C" JNIEXPORT void

JNICALL
Java_com_example_root_bumodemo_bumo_MainActivity_start(
        JNIEnv *env,
        jobject /* this */) {
    BumoApp::start();
}

extern "C" JNIEXPORT void

JNICALL
Java_com_example_root_bumodemo_bumo_MainActivity_stop(
        JNIEnv *env,
        jobject /* this */) {
    BumoApp::stop();
}