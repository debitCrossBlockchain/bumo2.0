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

#ifndef	_IFADDRS_H_
#define	_IFADDRS_H_

#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

// Android (bionic) doesn't have getifaddrs(3)/freeifaddrs(3).
// We fake it here, so java_net_NetworkInterface.cpp can use that API
// with all the non-portable code being in this file.

// Source-compatible subset of the BSD struct.
typedef struct ifaddrs {
    // Pointer to next struct in list, or NULL at end.
    struct ifaddrs* ifa_next;

    // Interface name.
    char* ifa_name;

    // Interface flags.
    unsigned int ifa_flags;

    // Interface network address.
    struct sockaddr* ifa_addr;

    // Interface netmask.
    struct sockaddr* ifa_netmask;
} ifaddrs;

#ifdef __cplusplus
extern "C" {
#endif
    int getifaddrs(ifaddrs** result);
    void freeifaddrs(ifaddrs* addresses);
#ifdef __cplusplus
}
#endif
#endif
