/*
  SDL_net:  An example cross-platform network library for use with SDL
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* Include normal system headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__OS2__) && !defined(__EMX__)
#include <nerrno.h>
#elif !defined(_WIN32_WCE)
#include <errno.h>
#endif

/* Include system network headers */
#if defined(__WIN32__) || defined(WIN32)
#define __USE_W32_SOCKETS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#else /* UNIX */
#if defined(__OS2__) || defined(__PSP__)
#include <sys/param.h>
#endif
#include <sys/types.h>
#ifdef __FreeBSD__
#include <sys/socket.h>
#endif
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#ifndef __BEOS__
#include <arpa/inet.h>
#endif
#ifdef __linux__ /* FIXME: what other platforms have this? */
#include <netinet/tcp.h>
#endif
#include <sys/socket.h>
#ifndef __PSP__
#include <net/if.h>
#endif
#include <netdb.h>
#endif /* WIN32 */

#ifdef __PSP__
#include <sys/select.h> /* for FD_SET, etc */
#endif

#ifdef __OS2__
typedef int socklen_t;
#elif defined(__USE_W32_SOCKETS) && !defined(IP_MSFILTER_SIZE)
typedef int socklen_t;
#elif 0
/* FIXME: What platforms need this? */
typedef Uint32 socklen_t;
#endif

/* System-dependent definitions */
#ifndef __USE_W32_SOCKETS
#ifdef __OS2__
#define closesocket     soclose
#else  /* !__OS2__ */
#define closesocket	close
#endif /* __OS2__ */
#define SOCKET	int
#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
#endif /* __USE_W32_SOCKETS */

#ifdef __USE_W32_SOCKETS
#define SDLNet_GetLastError WSAGetLastError
#define SDLNet_SetLastError WSASetLastError
#ifndef EINTR
#define EINTR WSAEINTR
#endif
#else
int SDLNet_GetLastError(void);
void SDLNet_SetLastError(int err);
#endif

