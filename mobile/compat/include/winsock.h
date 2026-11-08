// Cynex: winsock -> bsd soket
#pragma once
#include <windows.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

typedef int SOCKET;
typedef struct sockaddr SOCKADDR, *PSOCKADDR, *LPSOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN, *PSOCKADDR_IN, *LPSOCKADDR_IN;
typedef struct hostent HOSTENT, *PHOSTENT, *LPHOSTENT;
typedef struct in_addr IN_ADDR, *PIN_ADDR;
typedef struct timeval TIMEVAL;

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define SD_RECEIVE SHUT_RD
#define SD_SEND SHUT_WR
#define SD_BOTH SHUT_RDWR

#define WSABASEERR 10000
#define WSAEINTR EINTR
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINPROGRESS EINPROGRESS
#define WSAEALREADY EALREADY
#define WSAENOTSOCK ENOTSOCK
#define WSAEMSGSIZE EMSGSIZE
#define WSAEADDRINUSE EADDRINUSE
#define WSAENETDOWN ENETDOWN
#define WSAENETUNREACH ENETUNREACH
#define WSAECONNABORTED ECONNABORTED
#define WSAECONNRESET ECONNRESET
#define WSAENOBUFS ENOBUFS
#define WSAEISCONN EISCONN
#define WSAENOTCONN ENOTCONN
#define WSAETIMEDOUT ETIMEDOUT
#define WSAECONNREFUSED ECONNREFUSED
#define WSAEHOSTUNREACH EHOSTUNREACH

typedef struct WSAData
{
	WORD wVersion;
	WORD wHighVersion;
	char szDescription[257];
	char szSystemStatus[129];
	unsigned short iMaxSockets;
	unsigned short iMaxUdpDg;
	char* lpVendorInfo;
} WSADATA, *LPWSADATA;

inline int WSAStartup(WORD, LPWSADATA lpWSAData) { if (lpWSAData) memset(lpWSAData, 0, sizeof(WSADATA)); return 0; }
inline int WSACleanup() { return 0; }
inline int WSAGetLastError() { return errno; }
inline void WSASetLastError(int e) { errno = e; }
inline int closesocket(SOCKET s) { return close(s); }

// Cynex: FIONBIO icin fcntl daha guvenli
// Cynex: windows u_long 4 bayt - istemci DWORD veriyor
inline int ioctlsocket(SOCKET s, long cmd, DWORD* argp)
{
	if (cmd == (long)FIONBIO)
	{
		int flags = fcntl(s, F_GETFL, 0);
		if (flags < 0)
			return SOCKET_ERROR;
		flags = (argp && *argp) ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
		return fcntl(s, F_SETFL, flags) < 0 ? SOCKET_ERROR : 0;
	}
	int v = argp ? (int)*argp : 0;
	int r = ioctl(s, cmd, &v);
	if (argp)
		*argp = (DWORD)v;
	return r;
}

// Cynex: windows int* uzunluk aliyor, posix socklen_t*
inline ssize_t recvfrom(int s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen)
{ socklen_t l = fromlen ? (socklen_t)*fromlen : 0; ssize_t r = ::recvfrom(s, buf, (size_t)len, flags, from, fromlen ? &l : nullptr); if (fromlen) *fromlen = (int)l; return r; }
inline int accept(int s, struct sockaddr* addr, int* addrlen)
{ socklen_t l = addrlen ? (socklen_t)*addrlen : 0; int r = ::accept(s, addr, addrlen ? &l : nullptr); if (addrlen) *addrlen = (int)l; return r; }
inline int getsockname(int s, struct sockaddr* addr, int* addrlen)
{ socklen_t l = addrlen ? (socklen_t)*addrlen : 0; int r = ::getsockname(s, addr, addrlen ? &l : nullptr); if (addrlen) *addrlen = (int)l; return r; }
inline int getpeername(int s, struct sockaddr* addr, int* addrlen)
{ socklen_t l = addrlen ? (socklen_t)*addrlen : 0; int r = ::getpeername(s, addr, addrlen ? &l : nullptr); if (addrlen) *addrlen = (int)l; return r; }
inline int getsockopt(int s, int level, int optname, char* optval, int* optlen)
{ socklen_t l = optlen ? (socklen_t)*optlen : 0; int r = ::getsockopt(s, level, optname, optval, optlen ? &l : nullptr); if (optlen) *optlen = (int)l; return r; }

// Cynex: posix'te nfds=0 hicbir soketi denetlemez (windows yok sayar)
inline int cynex_select(int nfds, fd_set* r, fd_set* w, fd_set* e, struct timeval* t)
{
	return ::select(nfds > 0 ? nfds : FD_SETSIZE, r, w, e, t);
}
// Cynex: bloklamayan connect linux'ta EINPROGRESS, windows'ta WSAEWOULDBLOCK
inline int cynex_connect(int s, const struct sockaddr* addr, socklen_t len)
{
	const int r = ::connect(s, addr, len);
	if (r < 0 && errno == EINPROGRESS)
		errno = EWOULDBLOCK;
	return r;
}
#define select(n, r, w, e, t) cynex_select(n, r, w, e, t)
#define connect(s, a, l) cynex_connect(s, a, l)
