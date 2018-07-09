
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
	static int Bumoatoi(const std::string &str);
	static long long Bumoatoll(const std::string &str);
	static long  Bumoatol(const std::string &str);
	static double Bumoatof(const std::string &str);
	static void Bumoabort(void);
private:

};
#endif