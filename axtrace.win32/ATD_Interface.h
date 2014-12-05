/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#ifndef __AXTRACE_INTERFACE__
#define __AXTRACE_INTERFACE__

/** Init axtrace */
int __stdcall AxTrace_Init(const char* szTraceServer = 0, int nTracePort = 0);

#define AXTRACE_MAGIC_HEAD		(0x5841)	/* 'AX' */

/* AxTrace Communication Data Struct*/
typedef __declspec(align(1)) struct
{
	unsigned short	MAGIC_HEAD;		/* AXTRACE_MAGIC_HEAD ('AX') */
	unsigned short	wContentLen;	/* Content Length */
	unsigned int	dwProcessID;	/* Process ID*/
	unsigned int	dwThreadID;		/* Thread ID*/
	unsigned short	wTraceType;		/* Trace Type <TODO> */
	unsigned char	cWinID;			/* Trace Window ID */
	unsigned char	cStyleID;		/* Trace Style ID*/
} AXIATRACE_DATAHEAD;

typedef __declspec(align(16)) struct
{
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} AXIATRACE_TIME;

/** AxTrace Type enum*/
enum AxTraceType 
{
	ATT_LOG		= 1,	//String log
	ATT_VALUE	= 2,	//Value watch
};

/** */
enum AxTraceCodePage
{
	ATC_ACP		= 0,	//Default Windows ANSI code page.
	ATC_UTF8	= 1,	//Unicode 8
	ATC_UTF16	= 2,	//Unicode 16
};

/** AxTrace value type */
enum AXTRACE_VALUE_TYPE
{
	AX_INT8,
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
	AX_STR_UTF16,

	AX_MAX_VALUETYPE
};

/** AxTrace Const Value*/
enum
{
	AT_MaxWindowID = 0xFF,
	AT_MaxStyleID = 0xFF,
	AT_MaxLogLengthInByte = 0xFFFF-1,
	AT_MaxValueNameLength=128,
};

/** common cookie memory file name */
#define AXTRACE_COMMON_COOKIE_FILENAME	L"fff8c1cc-f42b-4303-842d-ced711479d8c"

/** Common cookie in memory */
typedef struct 
{
	unsigned int nListenPort;
	HWND hMainWnd;
} AXTRACE_COMMON_COOKIE;

/** Insert string log (acp/utf8,with null char as end) to log window*/
int __stdcall AxTrace_InsertLogA(int idWindow, int idStyle, int isUTF8, const char* stringBuf);

/** Insert wide string log (utf16, with null char as end) to log window*/
int __stdcall AxTrace_InsertLogW(int idWindow, int idStyle, const unsigned short* stringBuf);

/** */
int __stdcall AxTrace_WatchValue(int idWindow, int idStyle, int valueType, const char* name, int nameLengthInBytes, const void* value);

#endif
