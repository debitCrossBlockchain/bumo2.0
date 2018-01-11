#ifndef TEST_V8_TERMINATE_H_
#define TEST_V8_TERMINATE_H_

#include <v8.h>
#include <libplatform/libplatform.h>
#include <libplatform/libplatform-export.h>

bool test_v8(const char *path);
int main_v8(const char *path);
#endif