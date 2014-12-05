/***************************************************

                     AXIA|Trace3		
				
       (C) Copyright  Jean(www.thecodeway.com). 2013
***************************************************/

#ifndef __AXIA_TRACE__
#define __AXIA_TRACE__

#ifdef __cplusplus
	#define AXTRACE_EXTERN_C extern "C"
#else
	#define AXTRACE_EXTERN_C extern
#endif

typedef enum 
{
	AX_INT8=0,
	AX_UINT8,
	AX_INT16,
	AX_UINT16,
	AX_INT32,
	AX_UINT32,
	AX_INT64,
	AX_UINT64,
	AX_FLOAT32,
	AX_FLOAT64,
	AX_STR_ACP,
	AX_STR_UTF8,
	AX_STR_UTF16
} AXTRACE_VALUE_TYPE;

/* 
Finds the compiler type and version.
*/
#if defined(_MSC_VER)
	/*-------------------------*/
	/* Microsoft Visual Studio */
	/*-------------------------*/

	/* for UNICODE Application */
	#if !defined(_WCHAR_T_DEFINED)
		/* 
		Typically, wchar_t is defined when you use /Zc:wchar_t or 
		when typedef unsigned short wchar_t; is executed in code.
		*/
		typedef unsigned short wchar_t;
	#endif

	AXTRACE_EXTERN_C void AxTrace_Init(const char *szTraceServer, int nTracePort);
	AXTRACE_EXTERN_C void AxTrace(const char *psz_fmt, ...);
	AXTRACE_EXTERN_C void AxTraceExA(int win_id, int style_id, int is_utf8, const char *psz_fmt, ...);
	AXTRACE_EXTERN_C void AxTraceExW(int win_id, int style_id, const wchar_t *wsz_fmt, ...);

	AXTRACE_EXTERN_C void AxValue(const char *psz_name, AXTRACE_VALUE_TYPE type, const void* value);
	AXTRACE_EXTERN_C void AxValueEx(int win_id, int style_id, const char *psz_name, AXTRACE_VALUE_TYPE type, const void* value);

#elif defined(__GNUC__)
	/*      oops,    -_-!         */
	#pragma error "NOT SUPPORT YET!!"  
#elif defined(__BORLANDC__)
	/*      oops,    -_-!         */
	#pragma error "NOT SUPPORT YET!!"  
#else
	#pragma error "No known compiler. Abort! Abort!"
#endif

#endif /*__AXIA_TRACE__ */