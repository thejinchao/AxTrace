/*
		axia|trace3
	Copyright(C) thecodeway.com
*/
#ifndef __AXIA_TRACE_LINUX_INCLUDE__
#define __AXIA_TRACE_LINUX_INCLUDE__

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

#define ATC_UTF8	(1)	//Unicode 8
#define ATC_UTF16	(2)	//Unicode 16

#define AXV_INT8		(0)
#define AXV_UINT8		(1)
#define AXV_INT16		(2)
#define AXV_UINT16		(3)
#define AXV_INT32		(4)
#define AXV_UINT32		(5)
#define AXV_INT64		(6)
#define AXV_UINT64		(7)
#define AXV_FLOAT32		(8)
#define AXV_FLOAT64		(9)
#define AXV_STR_ACP		(10)
#define AXV_STR_UTF8	(11)
#define AXV_STR_UTF16	(12)
#define AXV_USER_DEF	(100)

/*
* send a log message to axtrace server
* @param style is one of AXT_*** value, 
* @param format is the message described with system current codec
*
*	sample: axlog(AXT_TRACE, "hello,world! I'm %s", name);
*/
AXTRACE_EXTERN_C void axlog(unsigned int style, const char *format, ...);

/*
* watch a value
* @param style is one of AXT_*** value,
* @param value_type is one of AXV_*** value,
* @param value_name is the name of value, ended with '\0'
* @param value is the memory address of value, the size of value is decided by value_type
*/
AXTRACE_EXTERN_C void axvalue(unsigned int style, unsigned int value_type, const char* value_name, const void* value);


#endif
