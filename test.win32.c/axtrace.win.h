/*
axia|trace3
Copyright(C) thecodeway.com
*/
#ifndef __AXIA_TRACE_WINDOWS_INCLUDE__
#define __AXIA_TRACE_WINDOWS_INCLUDE__

#ifdef __cplusplus
#define AXTRACE_EXTERN_C extern "C"
#else
#define AXTRACE_EXTERN_C extern
#endif

#define AXT_TRACE	(0)
#define AXT_DEBUG	(1)
#define AXT_INFO	(2)
#define AXT_WARN	(3)
#define AXT_ERROR	(4)
#define AXT_FATAL	(5)

#define AXT_USERDEF	(10)

/*
*finds the compiler type and version.
*/
#if defined(_MSC_VER)
/*-------------------------*/
/* Microsoft Visual Studio */
/*-------------------------*/

/* for UNICODE Application */
#if !defined(_WCHAR_T_DEFINED)
/*
*Typically, wchar_t is defined when you use /Zc:wchar_t or
*when typedef unsigned short wchar_t; is executed in code.
*/
typedef unsigned short wchar_t;
#endif

/* 
* set axtrace server address and port, 
* you don't need to call it if the server is 127.0.0.1:1978
* if not, you must call it before use any other functions.
*/
AXTRACE_EXTERN_C void axtrace_init(const char* server_ip, unsigned short server_port);

/*
* send a log message to axtrace server
* @param type is one of AXT_*** value, 
* @param format is the message described with system current codec
*
*	sample: axtrace(AXT_TRACE, "hello,world! I'm %s", name);
*/
AXTRACE_EXTERN_C void axtrace(unsigned int type, const char *format, ...);


#elif defined(__GNUC__)
/*      oops,    -_-!         */
#pragma error "NOT SUPPORT YET!!"  
#elif defined(__BORLANDC__)
/*      oops,    -_-!         */
#pragma error "NOT SUPPORT YET!!"  
#else
#pragma error "No known compiler."
#endif


#endif
