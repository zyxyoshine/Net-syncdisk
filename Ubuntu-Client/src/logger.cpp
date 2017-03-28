#include "logger.h"
#include <stdarg.h>
#include <time.h>
#include <stdio.h>

static char timebuf[20];
static struct tm sTm;
static time_t now;

static void updTime() {
	now = time(0);
#ifdef _WIN32
	gmtime_s(&sTm, &now);
#elif __linux__
    sTm = *gmtime(&now);
#endif
	strftime(timebuf, sizeof(timebuf), TIMEFORMAT, &sTm);
}

static void printMsg(
	const char *fname,
	int line,
	const char *func,
	MsgType msgType,
	bool sysErr, 
	int err, 
	const char *format, 
	va_list ap) {
		#define BUFSIZE 500
		char buf[BUFSIZE], errText[BUFSIZE], userMsg[BUFSIZE];
		
		updTime();
		vsnprintf(userMsg, BUFSIZE, format, ap);
		
		if (sysErr) {
#ifdef _WIN32
			static char _str[255];
			strerror_s(_str, sizeof(_str), err);
			snprintf(errText, BUFSIZE, "[%s]", _str);
#elif __linux__
			snprintf(errText, BUFSIZE, "[%s]", strerror(err));
#endif
		}
		else 
			snprintf(errText, BUFSIZE, ":");
		
		snprintf(buf, BUFSIZE, "%s %s %s():%d | %s%s %s\n", 
			timebuf, fname, func, line,
			msgType == TLOGERROR ? "ERROR" :
			msgType == TLOGLOG ? "LOG": 
			msgType == TLOGUSAGEERR ? "UsageErr" :
			msgType == TLOGFATAL ? "FATAL" : "UNKNOWN",
			errText, userMsg);
		fputs(buf, stderr);
		fflush(stderr);
	}
	
void __errExit(const char *fname, int line, const char *func, const char *format, ...) {
	va_list ap;
	int savedErrno = errno;
	va_start(ap, format);
	printMsg(fname, line, func, TLOGERROR, true, errno, format, ap);
	va_end(ap);
	errno = savedErrno;
	exit(EXIT_FAILURE);
}

void __fatal(const char *fname, int line, const char *func, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	printMsg(fname, line, func, TLOGFATAL, false, 0, format, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void __logger(const char *fname, int line, const char *func, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	printMsg(fname, line, func, TLOGLOG, false, 0, format, ap);
	va_end(ap);
}

void __usageErr(const char *fname, int line, const char *func, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	printMsg(fname, line, func, TLOGUSAGEERR, false, 0, format, ap);
	va_end(ap);
}
