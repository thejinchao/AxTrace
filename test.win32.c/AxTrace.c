/***************************************************

                     AXIA|Trace3		
				
       (C) Copyright  Jean(www.thecodeway.com). 2013
***************************************************/

#include "AxTrace.h"

#include <windows.h>
#include <strsafe.h>


/*-----------------------------------------------------*/
/*                  Global Data                        */
/*-----------------------------------------------------*/
#define AXTRACE_CONTENTS_LENGTH (2048)    /* MAX LENGTH IS 0xFFFF */
#define AXTRACE_VALUENAME_LENGTH (128)

/** Dll function */
typedef int(__stdcall* AXTRACE_DLLFUNC_INIT)(const char* szTraceServer, int nTracePort);
typedef int (__stdcall* AXTRACE_DLLFUNC_INSERTLOGA)(int idWindow, int idStyle, int isUTF8, const char* string);
typedef int (__stdcall* AXTRACE_DLLFUNC_INSERTLOGW)(int idWindow, int idStyle, const unsigned short* utf16_string);
typedef int (__stdcall* AXTRACE_DLLFUNC_WATEVALUE)(int idWindow, int idStyle, int valueType, const char* valueName, int valueNameLength, const void* value);

/* AxTrace Global data  */
typedef struct
{
	int		isInitSucc;		/* 0 means not, 1 means yes*/
	HMODULE	hAxTraceDll;

	AXTRACE_DLLFUNC_INIT		funcInit;
	AXTRACE_DLLFUNC_INSERTLOGA	funcInsertLogA;
	AXTRACE_DLLFUNC_INSERTLOGW	funcInsertLogW;
	AXTRACE_DLLFUNC_WATEVALUE	funcWatchValue;
} AXTRACE_GLOBAL_DATA, FAR *LPAXTRACE_GLOBAL_DATA;

/*---------------------------------------------------------------------------------------------*/
LPAXTRACE_GLOBAL_DATA _AxTrace_GetGlobalData(void)
{
	static AXTRACE_GLOBAL_DATA* s_theAxTraceGlobal = 0;
	LPAXTRACE_GLOBAL_DATA pTemp=0;

	/* is inited */
	if(s_theAxTraceGlobal!=0 ) return (s_theAxTraceGlobal->isInitSucc!=0 ? s_theAxTraceGlobal : 0);

	/* try init */
	pTemp = (LPAXTRACE_GLOBAL_DATA)LocalAlloc(LPTR, sizeof(AXTRACE_GLOBAL_DATA));

	pTemp->isInitSucc = 0;
	pTemp->hAxTraceDll = LoadLibraryA("axtrace.dll");
	if(pTemp->hAxTraceDll)
	{
		pTemp->funcInit = (AXTRACE_DLLFUNC_INIT)GetProcAddress(pTemp->hAxTraceDll, "AxTrace_Init");
		pTemp->funcInsertLogA = (AXTRACE_DLLFUNC_INSERTLOGA)GetProcAddress(pTemp->hAxTraceDll, "AxTrace_InsertLogA");
		pTemp->funcInsertLogW = (AXTRACE_DLLFUNC_INSERTLOGW)GetProcAddress(pTemp->hAxTraceDll, "AxTrace_InsertLogW");
		pTemp->funcWatchValue = (AXTRACE_DLLFUNC_WATEVALUE)GetProcAddress(pTemp->hAxTraceDll, "AxTrace_WatchValue");
	}

	/* is all function load ok? */
	if(	pTemp->hAxTraceDll!=0 && 
		pTemp->funcInit!=0 && 
		pTemp->funcInsertLogA!=0 && pTemp->funcInsertLogW!=0 ) 
	{
		pTemp->isInitSucc = 1;
	}

	/* apply */
	InterlockedCompareExchangePointer((PVOID volatile *)&s_theAxTraceGlobal, pTemp, 0);
	if(pTemp != s_theAxTraceGlobal) /* already applyed? */
	{
		LocalFree(pTemp);
	}
	return ((s_theAxTraceGlobal && s_theAxTraceGlobal->isInitSucc!=0) ? s_theAxTraceGlobal : 0);
}

/*---------------------------------------------------------------------------------------------*/
void AxTrace_Init(const char *szTraceServer, int nTracePort)
{
	LPAXTRACE_GLOBAL_DATA pGlobalData = 0;

	pGlobalData = _AxTrace_GetGlobalData();
	if (pGlobalData == 0) return;

	pGlobalData->funcInit(szTraceServer, nTracePort);
}

/*---------------------------------------------------------------------------------------------*/
void AxTrace(const char *psz_fmt, ...)
{
	LPAXTRACE_GLOBAL_DATA pGlobalData = 0;
	va_list ptr=0;
	HRESULT hr;
	char szContents[AXTRACE_CONTENTS_LENGTH];
	size_t contentsSizeInBytes;
	
	/* is init ok? */
	pGlobalData = _AxTrace_GetGlobalData();
	if(pGlobalData==0) return;

	/* Create String Contents*/
	va_start(ptr, psz_fmt);
	hr = StringCbVPrintfA(szContents, AXTRACE_CONTENTS_LENGTH, psz_fmt, ptr);
	va_end(ptr);

	/* failed ?*/
	if(FAILED(hr)) return;

	/** get string length*/
	hr = StringCbLengthA(szContents, AXTRACE_CONTENTS_LENGTH-1, &contentsSizeInBytes);
	if(FAILED(hr)) return;

	/* Call Real Send Function */
	pGlobalData->funcInsertLogA(0, 0, 0, szContents);
}

/*---------------------------------------------------------------------------------------------*/
void AxTraceExA(int win_id, int style_id, int is_utf8, const char *psz_fmt, ...)
{
	LPAXTRACE_GLOBAL_DATA pGlobalData = 0;
	va_list ptr=0;
	HRESULT hr;
	char szContents[AXTRACE_CONTENTS_LENGTH];
	size_t contentsSizeInBytes;
	
	/* is init ok? */
	pGlobalData = _AxTrace_GetGlobalData();
	if(pGlobalData==0) return;

	/* Create String Contents*/
	va_start(ptr, psz_fmt);
	hr = StringCbVPrintfA(szContents, AXTRACE_CONTENTS_LENGTH, psz_fmt, ptr);
	va_end(ptr);

	/* failed ?*/
	if(FAILED(hr)) return;

	/** get string length*/
	hr = StringCbLengthA(szContents, AXTRACE_CONTENTS_LENGTH-1, &contentsSizeInBytes);
	if(FAILED(hr)) return;

	/* Call Real Send Function */
	pGlobalData->funcInsertLogA(win_id, style_id, (is_utf8!=0 ? 1 : 0), szContents);
}

/*---------------------------------------------------------------------------------------------*/
void AxTraceExW(int win_id, int style_id, const wchar_t *wsz_fmt, ...)
{
	LPAXTRACE_GLOBAL_DATA pGlobalData = 0;
	va_list ptr=0;
	HRESULT hr;
	wchar_t wszContents[AXTRACE_CONTENTS_LENGTH];
	size_t contentsSizeInBytes;
	
	/* is init ok? */
	pGlobalData = _AxTrace_GetGlobalData();
	if(pGlobalData==0) return;

	/* Create String Contents*/
	va_start(ptr, wsz_fmt);
	hr = StringCbVPrintfW(wszContents, AXTRACE_CONTENTS_LENGTH, wsz_fmt, ptr);
	va_end(ptr);

	/* failed ?*/
	if(FAILED(hr)) return;

	/** get string length*/
	hr = StringCbLengthW(wszContents, AXTRACE_CONTENTS_LENGTH-1, &contentsSizeInBytes);
	if(FAILED(hr)) return;

	/* Call Real Send Function */
	pGlobalData->funcInsertLogW(win_id, style_id, (const unsigned short*)wszContents);
}

/*---------------------------------------------------------------------------------------------*/
void AxValueEx(int win_id, int style_id, const char *psz_name, AXTRACE_VALUE_TYPE type, const void* value)
{
	LPAXTRACE_GLOBAL_DATA pGlobalData = 0;
	HRESULT hr;
	size_t valueNameLength;

	/* is init ok? */
	pGlobalData = _AxTrace_GetGlobalData();
	if(pGlobalData==0) return;

	/** get value name length*/
	hr = StringCbLengthA(psz_name, AXTRACE_VALUENAME_LENGTH-1, &valueNameLength);
	if(FAILED(hr)) return;

	/* Call Real Send Function */
	pGlobalData->funcWatchValue(win_id, style_id, (int)type, psz_name, (int)valueNameLength, value);
}

/*---------------------------------------------------------------------------------------------*/
void AxValue(const char *psz_name, AXTRACE_VALUE_TYPE type, const void* value)
{
	AxValueEx(0, 0, psz_name, type, value);
}

