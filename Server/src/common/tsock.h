#pragma once
#include <stdio.h>
enum TsockErr {
	TOK = 0, TTIMEOUT, TERROR, TCLOSE, TNOSESSION
};

#ifdef _WIN32

#define _CRT_NONSTDC_NO_WARNINGS

#include <ws2tcpip.h>
#include <Winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

void initsock();
void setnb(int fd, bool block = false);
void tclose(int fd);
int terrno();

#define TEINTR WSAEINTR
#define TEBADF WSAEBADF
#define TEACCES WSAEACCES
#define TEFAULT WSAEFAULT
#define TEINVAL WSAEINVAL
#define TEMFILE WSAEMFILE
#define TEWOULDBLOCK WSAEWOULDBLOCK
#define TEINPROGRESS WSAEINPROGRESS
#define TEALREADY WSAEALREADY
#define TENOTSOCK WSAENOTSOCK
#define TEDESTADDRREQ WSAEDESTADDRREQ
#define TEMSGSIZE WSAEMSGSIZE
#define TEPROTOTYPE WSAEPROTOTYPE
#define TENOPROTOOPT WSAENOPROTOOPT
#define TEPROTONOSUPPORT WSAEPROTONOSUPPORT
#define TESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
#define TEOPNOTSUPP WSAEOPNOTSUPP
#define TEPFNOSUPPORT WSAEPFNOSUPPORT
#define TEAFNOSUPPORT WSAEAFNOSUPPORT
#define TEADDRINUSE WSAEADDRINUSE
#define TEADDRNOTAVAIL WSAEADDRNOTAVAIL
#define TENETDOWN WSAENETDOWN
#define TENETUNREACH WSAENETUNREACH
#define TENETRESET WSAENETRESET
#define TECONNABORTED WSAECONNABORTED
#define TECONNRESET WSAECONNRESET
#define TENOBUFS WSAENOBUFS
#define TEISCONN WSAEISCONN
#define TENOTCONN WSAENOTCONN
#define TESHUTDOWN WSAESHUTDOWN
#define TETOOMANYREFS WSAETOOMANYREFS
#define TETIMEDOUT WSAETIMEDOUT
#define TECONNREFUSED WSAECONNREFUSED
#define TELOOP WSAELOOP
#define TENAMETOOLONG WSAENAMETOOLONG
#define TEHOSTDOWN WSAEHOSTDOWN
#define TEHOSTUNREACH WSAEHOSTUNREACH
#define TENOTEMPTY WSAENOTEMPTY
#define TEPROCLIM WSAEPROCLIM
#define TEUSERS WSAEUSERS
#define TEDQUOT WSAEDQUOT
#define TESTALE WSAESTALE
#define TEREMOTE WSAEREMOTE
#define SYSNOTREADY WSASYSNOTREADY
#define VERNOTSUPPORTED WSAVERNOTSUPPORTED
#define NOTINITIALISED WSANOTINITIALISED
#define TEDISCON WSAEDISCON
#define TENOMORE WSAENOMORE
#define TECANCELLED WSAECANCELLED
#define TEINVALIDPROCTABLE WSAEINVALIDPROCTABLE
#define TEINVALIDPROVIDER WSAEINVALIDPROVIDER
#define TEPROVIDERFAILEDINIT WSAEPROVIDERFAILEDINIT
#define SYSCALLFAILURE WSASYSCALLFAILURE
#define SERVICE_NOT_FOUND WSASERVICE_NOT_FOUND
#define TYPE_NOT_FOUND WSATYPE_NOT_FOUND
#define _E_NO_MORE WSA_E_NO_MORE
#define _E_CANCELLED WSA_E_CANCELLED
#define TEREFUSED WSAEREFUSED
#define HOST_NOT_FOUND WSAHOST_NOT_FOUND
#define TRY_AGAIN WSATRY_AGAIN
#define NO_RECOVERY WSANO_RECOVERY
#define NO_DATA WSANO_DATA


#elif __linux__ 

#define FORLINUX 2

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>	
#include <errno.h>

#define TEINTR EINTR 
#define TEBADF EBADF 
#define TEACCES EACCES 
#define TEFAULT EFAULT 
#define TEINVAL EINVAL 
#define TEMFILE EMFILE 
#define TEWOULDBLOCK EWOULDBLOCK 
#define TEINPROGRESS EINPROGRESS 
#define TEALREADY EALREADY 
#define TENOTSOCK ENOTSOCK 
#define TEDESTADDRREQ EDESTADDRREQ 
#define TEMSGSIZE EMSGSIZE 
#define TEPROTOTYPE EPROTOTYPE 
#define TENOPROTOOPT ENOPROTOOPT 
#define TEPROTONOSUPPORT EPROTONOSUPPORT 
#define TESOCKTNOSUPPORT ESOCKTNOSUPPORT 
#define TEOPNOTSUPP EOPNOTSUPP 
#define TEPFNOSUPPORT EPFNOSUPPORT 
#define TEAFNOSUPPORT EAFNOSUPPORT 
#define TEADDRINUSE EADDRINUSE 
#define TEADDRNOTAVAIL EADDRNOTAVAIL 
#define TENETDOWN ENETDOWN 
#define TENETUNREACH ENETUNREACH 
#define TENETRESET ENETRESET 
#define TECONNABORTED ECONNABORTED 
#define TECONNRESET ECONNRESET 
#define TENOBUFS ENOBUFS 
#define TEISCONN EISCONN 
#define TENOTCONN ENOTCONN 
#define TESHUTDOWN ESHUTDOWN 
#define TETOOMANYREFS ETOOMANYREFS 
#define TETIMEDOUT ETIMEDOUT 
#define TECONNREFUSED ECONNREFUSED 
#define TELOOP ELOOP 
#define TENAMETOOLONG ENAMETOOLONG 
#define TEHOSTDOWN EHOSTDOWN 
#define TEHOSTUNREACH EHOSTUNREACH 
#define TENOTEMPTY ENOTEMPTY 
#define TEPROCLIM EPROCLIM 
#define TEUSERS EUSERS 
#define TEDQUOT EDQUOT 
#define TESTALE ESTALE 
#define TEREMOTE EREMOTE 
#define SYSNOTREADY 
#define VERNOTSUPPORTED ED 
#define NOTINITIALISED 
#define TEDISCON EDISCON 
#define TENOMORE ENOMORE 
#define TECANCELLED ECANCELLED 
#define TEINVALIDPROCTABLE EINVALIDPROCTABLE 
#define TEINVALIDPROVIDER EINVALIDPROVIDER 
#define TEPROVIDERFAILEDINIT EPROVIDERFAILEDINIT 

#define terrno() errno

void setnb(int fd, bool block = false);

void tclose(int fd);

#endif

typedef struct sockaddr_in SA4;
typedef struct sockaddr SA;
#define TREAD 1
#define TWRITE 2

static timeval _default_to = { 1, 0 }, *_default_to_p = &_default_to;

inline int blockt(int fd, int io, timeval *timeout = _default_to_p) {

	fd_set rset, wset, *_rset = NULL, *_wset = NULL;
	
	if (io & TREAD) {
		FD_ZERO(&rset); 
		FD_SET(fd, &rset);
		_rset = &rset;
	}

	if (io & TWRITE) {
		FD_ZERO(&wset);
		FD_SET(fd, &wset);
		_wset = &wset;
	}

	select(fd + 1, _rset, _wset, NULL, timeout);

	int flags = (io & TREAD) && FD_ISSET(fd, _rset) ? TREAD : 0;
	flags |= (io & TWRITE) && FD_ISSET(fd, _wset) ? TWRITE : 0;
	return flags;
}

inline int sendn(int fd, const char *buf, int len, bool blocking = 1, timeval *timeout = _default_to_p) {
	int n = 0, m = 0;
	while (n < len) {
		if(!blocking)
			if (!(blockt(fd, TWRITE, timeout) | TWRITE))
				return TTIMEOUT;
		if ((m = send(fd, buf + n, len - n, 0)) < 0)
			return -1;
		n += m;
	}
	return 0;
}

inline int readn(int fd, char *buf, int len, bool blocking = 1, timeval *timeout = _default_to_p) {
	int n = 0, m = 0;
	while (n < len) {
		if (!blocking)
			if (!(blockt(fd, TREAD, timeout) | TREAD))
				return TTIMEOUT;
		//printf("readn n = %d, len = %d\n", n, len);
		if ((m = recv(fd, buf + n, len - n, 0)) < 0) {
			auto err = terrno();
			return -1;
		}
		n += m;
	}
	return 0;
}

