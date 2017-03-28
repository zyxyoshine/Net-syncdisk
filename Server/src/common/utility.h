#pragma once
#include <vector>
#include "net.h"
#include <string>
#include "logger.h"

template <int const N>
class Readbuf {
private:
    int fd;
	char lastdet;
    char buf[N + 1];	
    char *t;
public:
    Readbuf(int fd_);
	/* return len */
    int readto(char *b, char det = '\0');
	/* return 0 for success */
	int readn(char *b, int len);
    //int readn(char *b, int len);

    bool blocking;
    timeval timeout;
};

template <int N>
Readbuf<N>::Readbuf(int fd_) {
	lastdet = -1;
	blocking = 1;
	timeout = { 1, 0 };
	t = buf;
	fd = fd_;
}

template <int N>
int Readbuf<N>::readn(char *b, int len) {	
	int n = t - buf;
	logger("%d bytes in Readbuf, %d bytes to read", n, len);
	if (n >= len) {
		memcpy(b, buf, len);
		memmove(buf, buf + len, n - len);
		t = buf + n - len;
	}
	else {
		int m = len - n;
		/* __fix this__ */
		memcpy(b, buf, n);
		if (::readn(fd, b + n, m) < 0)
			return -1;
		t = buf;
	}
	return 0;
}

template <int N>
int Readbuf<N>::readto(char *b, char det) {

	if (t-buf > 0) {
		int n = t - buf;
		char *p = buf;
		bool ok = false;
		while (n > 0) {
			if (*p == det) {
				++p; --n;
				ok = true;
				break;
			}
			++p; --n;
		}
		if (ok) {
			int len = p - buf;
			memcpy(b, buf, len);
			memmove(buf, p, n);
			t = buf + n;
			return len;
		}
	}
	lastdet = det;

	while (t-buf < N / 2) {

		int flags;
		if (!blocking) {

#ifdef DEBUG
			timeval tv = { 30, 0 };
			flags = blockt(fd, TREAD, &tv);
#else
			flags = blockt(fd, TREAD);
#endif

			if (!(flags & TREAD)) {
				return -1;
			}
		}

		int n = recv(fd, t, N - (t - buf), 0);

		if (n <= 0) {
			return n;
		}
		bool ok = false;
		while (n > 0) {
			if (*t == det) {
				++t; --n;
				ok = true;
				break;
			}
			++t; --n;
		}
		if (ok) {
			int len = t - buf;
			memcpy(b, buf, len);
			memmove(buf, t, n);
			t = buf + n;
			return len;
		}
	}

	int len = t - buf;
	memcpy(b, buf, len);
	t = buf;

	return len;
}

#define READBUFN (1024*1024)
typedef Readbuf<READBUFN> Readbuf_;

inline std::string time2str(const time_t &t) {
#ifdef _WIN32
	char buf[50];
	tm tm_;
	localtime_s(&tm_, &t);
	strftime(buf, 50, "%Y-%m-%dT%H:%M:%SZ", &tm_);
	return std::string(buf);
#elif __linux__
	char buff[50];
	strftime(buff, 50, "%Y-%m-%dT%H:%M:%SZ", localtime(&t));
#endif
}