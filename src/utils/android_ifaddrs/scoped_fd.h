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
#ifndef SCOPED_FD_H_included
#define SCOPED_FD_H_included

#ifdef OS_ANDROID

#include <unistd.h>

// A smart pointer that closes the given fd on going out of scope.
// Use this when the fd is incidental to the purpose of your function,
// but needs to be cleaned up on exit.
class ScopedFd {
public:
    explicit ScopedFd(int fd) : fd(fd) {
    }

    ~ScopedFd() {
        close(fd);
    }

    int get() const {
        return fd;
    }

private:
    int fd;

    // Disallow copy and assignment.
    ScopedFd(const ScopedFd&);
    void operator=(const ScopedFd&);
};

#endif

#endif  // SCOPED_FD_H_included