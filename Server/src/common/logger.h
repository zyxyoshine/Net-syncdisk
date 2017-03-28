#ifndef __LOGGER_H
#define __LOGGER_H

#if defined(__STDC__)
# define C89
# if defined(__STDC_VERSION__)
#  define C90
#  if (__STDC_VERSION__ >= 199409L)
#   define C94
#  endif
#  if (__STDC_VERSION__ >= 199901L)
#   define C99
#  endif
# endif
#endif

#ifdef C99
#define __FUNC__ __func__
#else
#ifdef C89
#define __FUNC__ __FUNCTION__
#endif
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#include <unistd.h>
#endif
#include <errno.h>
#include <string.h>

//#ifdef __linux__
//#endif

typedef enum { TLOGERROR, TLOGLOG, TLOGFATAL, TLOGUSAGEERR } MsgType;

#ifndef min

#define min(a,b) ((a) < (b) : (a) : (b))
#define max(a,b) ((a) > (b) : (a) : (b))

#endif

#ifdef __GUNC__ /* prevent 'gcc -Wall' from complaining of terminating non-void function with errHandling function */
	#define NORETURN __attribute__ ((__noreturn__))
#else 
	#define NORETURN
#endif

#define TIMEFORMAT "%H:%M:%S"

#ifdef _WIN32
#define __FUNC__ __FUNCTION__
#endif

#define errExit(...) __errExit(__FILE__, __LINE__, __FUNC__, __VA_ARGS__)
#define fatal(...) __fatal(__FILE__, __LINE__, __FUNC__, __VA_ARGS__)
#define usageErr(...) __usageErr(__FILE__, __LINE__, __FUNC__, __VA_ARGS__)
#define logger(...) __logger(__FILE__, __LINE__, __FUNC__, __VA_ARGS__)

void __errExit(const char *fname, int line, const char *func, const char *format, ...) NORETURN; /* syscall err, prints err according to errno */
void __fatal(const char *fname, int line, const char *func, const char *format, ...) NORETURN; /* logic err */
void __usageErr(const char *fname, int line, const char *func, const char *format, ...) NORETURN; /* usage err */
void __logger(const char *fname, int line, const char *func, const char *format, ...) NORETURN; /* flow logger */

#endif