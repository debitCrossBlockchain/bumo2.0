/*
	bumo is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	bumo is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with bumo.  If not, see <http://www.gnu.org/licenses/>.
*/
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
	 // Disallow copy and assignment.
	 Stdlib(const Stdlib&);
	 void operator=(const Stdlib&);

};
#endif