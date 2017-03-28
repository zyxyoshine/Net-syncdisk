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

#include <unistd.h>
#include <errno.h>
#include <string.h>

typedef enum { TRUE = 1, FALSE = 0 } Bool;
typedef enum { ERROR, LOG, FATAL, USAGEERR } MsgType;

#define min(a,b) ((a) < (b) : (a) : (b))
#define max(a,b) ((a) > (b) : (a) : (b))

#ifdef __GUNC__ /* prevent 'gcc -Wall' from complaining of terminating non-void function with errHandling function */
	#define NORETURN __attribute__ ((__noreturn__))
#else 
	#define NORETURN
#endif

#define TIMEFORMAT "%H:%M:%S"

#define errExit(...) __errExit(__FILE__, __LINE__, __FUNC__, __VA_ARGS__)
#define fatal(...) __fatal(__FILE__, __LINE__, __FUNC__, __VA_ARGS__)
#define usageErr(...) __usageErr(__FILE__, __LINE__, __FUNC__, __VA_ARGS__)
#define logger(...) __logger(__FILE__, __LINE__, __FUNC__, __VA_ARGS__)
//#define log2file(...) __log2file(__FILE__, __LINE__, __FUNC__, __VA_ARGS__)

void __errExit(const char *fname, int line, const char *func, const char *format, ...) NORETURN; /* syscall err, prints err according to errno */
void __fatal(const char *fname, int line, const char *func, const char *format, ...) NORETURN; /* logic err */
void __usageErr(const char *fname, int line, const char *func, const char *format, ...) NORETURN; /* usage err */
void __logger(const char *fname, int line, const char *func, const char *format, ...) NORETURN; /* flow logger */
//void __log2file(const char *fname, int line, const char *func, const char *format, ...) NORETURN; /* flow logger */

#endif