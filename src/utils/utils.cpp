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

#include "utils.h"
#include "file.h"
#include "strings.h"
#include "base_int.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <termios.h>
#endif

uint32_t utils::error_code() {
#ifdef WIN32
	return (uint32_t)GetLastError();
#else
	return (uint32_t)errno;
#endif //WIN32
}

void utils::set_error_code(uint32_t code) {
#ifdef WIN32
	SetLastError(code);
#else
	errno = code;
#endif //WIN32
}

void utils::Sleep(int time_milli) {
#ifdef WIN32
	::Sleep(time_milli);
#elif defined OS_LINUX
	::usleep(((__useconds_t)time_milli) * 1000);
#elif defined OS_MAC
	::usleep(time_milli * 1000);
#endif //WIN32
}

std::string utils::error_desc(uint32_t code) {
	uint32_t real_code = (((uint32_t)-1) == code) ? error_code() : code;
#ifdef WIN32
	LPVOID msg_buffer = NULL;
	if (!FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		real_code,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // Default language
		(LPSTR)&msg_buffer,
		0,
		NULL)) {
		// Handle the error.
		return std::string("");
	}

	// trim end blank characters
	char *msg_data = (char *)msg_buffer;
	int msg_len = (int)strlen(msg_data);
	while (msg_len > 0 && isspace(msg_data[msg_len - 1])) {
		msg_data[msg_len - 1] = 0;
		msg_len--;
	}

	std::string strError((const char *)msg_buffer);

	LocalFree(msg_buffer);

	return strError;

#else
	return std::string(strerror(real_code));
#endif
}

size_t utils::GetCpuCoreCount() {
	size_t core_count = 1;
#if defined(WIN32)
	SYSTEM_INFO nSystemInfo;
	GetSystemInfo(&nSystemInfo);
	core_count = nSystemInfo.dwNumberOfProcessors;
#else
	do {
		utils::File nProcFile;

		if (!nProcFile.Open("/proc/stat", utils::File::FILE_M_READ)) {
			break;
		}

		std::string strLine;

		if (!nProcFile.ReadLine(strLine, 1024)) {
			nProcFile.Close();
			break;
		}
		utils::StringVector nValues = utils::String::Strtok(strLine, ' ');
		if (nValues.size() < 8) {
			break;
		}

		core_count = nValues.size();
	} while (false);
#endif
	return core_count;
}

time_t utils::GetStartupTime(time_t time_now) {
	time_t nStartupTime = 0;

	if (0 == time_now) {
		time_now = time(NULL);
	}

#if defined(WIN32)
	LARGE_INTEGER nCount, nFreq;

	if (!QueryPerformanceCounter(&nCount) || !QueryPerformanceFrequency(&nFreq) || 0 == nFreq.QuadPart) {
		return 0;
	}

	nStartupTime = time_now - (time_t)(nCount.QuadPart / nFreq.QuadPart);
#elif defined OS_LINUX
	struct sysinfo nInfo;

	memset(&nInfo, 0, sizeof(nInfo));
	sysinfo(&nInfo);
	nStartupTime = time_now - (time_t)nInfo.uptime;

	//Utils::File nProcFile;

	//if( !nProcFile.Open("/proc/uptime", Utils::File::FILE_M_READ) )
	//{
	//	return 0;
	//}

	//std::string strLine;
	//Utils::StringArray nValues;

	//if( !nProcFile.ReadLine(strLine, 1024) || Utils::String::Split(strLine, nValues, ' ', -1, true) < 1 )
	//{
	//	nProcFile.Close();
	//	return 0;
	//}
	//nProcFile.Close();

	//uint32 nTimeSecs = Utils::String::ParseNumber(nValues[0], (uint32)0);
	//nStartupTime = nTimeNow - (time_t)nTimeSecs;
#elif defined OS_MAC
	struct timeval boottime;
	size_t len = sizeof(boottime);
	int mib[2] = { CTL_KERN, KERN_BOOTTIME };
	if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0)
	{
		return -1.0;
	}
	nStartupTime = boottime.tv_sec;
#endif

	return nStartupTime;
}

std::string utils::GetCinPassword(const std::string &_prompt) {
#if defined(_WIN32)
	std::cout << _prompt << std::flush;
	// Get current Console input flags
	HANDLE hStdin;
	DWORD fdwSaveOldMode;
	if ((hStdin = GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE)
		abort();
	if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
		abort();
	// Set console flags to no echo
	if (!SetConsoleMode(hStdin, fdwSaveOldMode & (~ENABLE_ECHO_INPUT)))
		abort();
	// Read the string
	std::string ret;
	std::getline(std::cin, ret);
	// Restore old input mode
	if (!SetConsoleMode(hStdin, fdwSaveOldMode))
		abort();
	return ret;
#else
	struct termios oflags;
	struct termios nflags;
	char password[256];

	// disable echo in the terminal
	tcgetattr(fileno(stdin), &oflags);
	nflags = oflags;
	nflags.c_lflag &= ~ECHO;
	nflags.c_lflag |= ECHONL;

	if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0)
		abort();

	printf("%s", _prompt.c_str());
	if (!fgets(password, sizeof(password), stdin))
		abort();
	password[strlen(password) - 1] = 0;

	// restore terminal
	if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0)
		abort();


	return password;
#endif
}

#ifdef OS_LINUX
extern "C"
{
	void * __wrap_memcpy(void *dest, const void *src, size_t n) {
		asm(".symver memcpy, memcpy@GLIBC_2.2.5");
		return memcpy(dest, src, n);
	}
}
#elif defined OS_MAC
#endif