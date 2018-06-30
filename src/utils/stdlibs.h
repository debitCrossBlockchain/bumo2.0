
#ifndef UTILS_STDLIBS_UTIL_H_
#define UTILS_STDLIBS_UTIL_H_
#include<cstdlib>

#ifdef OS_ANDROID
#define FMT_I64 "%lld"
#define FMT_I64_EX(fmt) "%" #fmt "lld"
#define FMT_U64 "%llu"
#define FMT_X64 "%llX"
#define FMT_SIZE "%u"
#endif

class Stdlib
{
public:
	Stdlib();
	~Stdlib();
	static int atoi(const char *_Str);
	static long long atoll(const char *_Str);
	static long  atol(const char *_Str);
	static double atof(const char *_String);
	static void abort(void);
private:

};
#endif