/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#include "StdAfx.h"
#include "AT_Config.h"
#include "AT_Util.h"
#include "utf/ConvertUTF.h"

namespace AT3
{
/** Regist key 
*/
const TCHAR* g_szAxTrace3Key = _T("SOFTWARE\\AXIA\\AxTrace3");

//--------------------------------------------------------------------------------------------
Config::Config()
	: m_hFont(0)
{
	// reset setting to default
	_resetDefaultSetting();
}

//--------------------------------------------------------------------------------------------
Config::Config(const Config& other)
	: m_hFont(0)
{
	// reset setting to default
	_resetDefaultSetting();
	copyFrom(other);
}

//--------------------------------------------------------------------------------------------
Config::~Config()
{
	if(m_hFont) ::DeleteObject(m_hFont);
}

//--------------------------------------------------------------------------------------------
void Config::_resetDefaultSetting(void)
{
	m_bCapture = true;
	m_bAutoscroll = true;

	m_bAlwaysOnTop = false;
	m_bShowMilliseconds = true;

	for(int i=0; i<MAX_TRACE_STYLE_COUNTS; i++)
	{
		m_allTraceStyle[i].useDefault=true;
	}
	
	if(m_hFont) ::DeleteObject(m_hFont);

	LOGFONT lf={0};
	::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
	m_hFont = CreateFontIndirect(&lf);

	//set default
	m_allTraceStyle[0].useDefault = false;
	m_allTraceStyle[0].colBak = GetSysColor(COLOR_WINDOW);
	m_allTraceStyle[0].colFront = GetSysColor(COLOR_WINDOWTEXT);
}

//--------------------------------------------------------------------------------------------
void Config::copyFrom(const Config& other)
{
	m_bCapture = other.m_bCapture;
	m_bAutoscroll = other.m_bAutoscroll;
	m_bAlwaysOnTop = other.m_bAlwaysOnTop;
	m_bShowMilliseconds = other.m_bShowMilliseconds;

	if(m_hFont) ::DeleteObject(m_hFont);
	LOGFONT lf={0};
	GetObject(other.m_hFont, sizeof(lf),&lf);
	m_hFont = CreateFontIndirect(&lf);

	memcpy(m_allTraceStyle, other.m_allTraceStyle, sizeof(m_allTraceStyle));
}

//--------------------------------------------------------------------------------------------
void Config::loadSetting(void)
{
	_resetDefaultSetting();

	DWORD type = REG_DWORD;
	DWORD size = sizeof(DWORD);
	DWORD dwTemp;
	TCHAR wszTemp[MAX_PATH] = { 0 };

	#define LOAD_BOOL_FROM_REG(param, name) \
		if(ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, g_szAxTrace3Key, name, &type, &dwTemp, &size)) \
		{ \
			param = (dwTemp!=0); \
		}

	LOAD_BOOL_FROM_REG(m_bAlwaysOnTop,		_T("AlwaysOnTop"));
	LOAD_BOOL_FROM_REG(m_bShowMilliseconds,	_T("ShowMillisenconds"));

	//get font 
	LOGFONT lf;
	type=REG_BINARY;
	size=sizeof(lf);
	if(ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, g_szAxTrace3Key, _T("Font"), &type, &lf, &size))
	{
		setFont(&lf);
	}

	// Trace style fold
	TCHAR szTraceStyle[MAX_PATH];
	StringCchPrintfW(szTraceStyle, MAX_PATH, _T("%s\\TraceStyle"), g_szAxTrace3Key);

	// Load trace style
	for(int i=0; i<MAX_TRACE_STYLE_COUNTS; i++)
	{
		TCHAR szName[MAX_PATH];
		StringCchPrintfW(szName, MAX_PATH, _T("Style%d"), i);

		type = REG_SZ;
		size = MAX_PATH;

		if(ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, szTraceStyle, szName, &type, wszTemp, &size))
		{
			COLORREF colFront, colBack;
			swscanf_s(wszTemp, _T("%08X|%08X"), &colFront, &colBack);

			m_allTraceStyle[i].useDefault = false;
			m_allTraceStyle[i].colBak = colBack;
			m_allTraceStyle[i].colFront = colFront;
		}
	}
}

//--------------------------------------------------------------------------------------------
void Config::saveSetting(void) const
{
	DWORD dwTemp;

	// Save dword value to regist
	#define SAVE_BOOL_TO_REG(param, name) \
		dwTemp = (param) ? 1 : 0; \
		SHSetValue(HKEY_CURRENT_USER, g_szAxTrace3Key, \
			name, REG_DWORD, &dwTemp, sizeof(DWORD));

	SAVE_BOOL_TO_REG(m_bAlwaysOnTop,		_T("AlwaysOnTop"));
	SAVE_BOOL_TO_REG(m_bShowMilliseconds,	_T("ShowMillisenconds"));

	//save font data
	LOGFONT lf;
	GetObject(m_hFont, sizeof(lf), &lf);
	SHSetValue(HKEY_CURRENT_USER, g_szAxTrace3Key, _T("Font"), REG_BINARY, &lf, sizeof(lf));

	// Trace style fold
	TCHAR szTraceStyle[MAX_PATH];
	StringCchPrintfW(szTraceStyle, MAX_PATH, _T("%s\\TraceStyle"), g_szAxTrace3Key);

	// Delete old style
	SHDeleteKey(HKEY_CURRENT_USER, szTraceStyle);

	// Save trace style
	for(int i=0; i<MAX_TRACE_STYLE_COUNTS; i++)
	{
		const TraceStyle& theStyle = m_allTraceStyle[i];
		if(theStyle.useDefault) continue;

		TCHAR szName[MAX_PATH];
		StringCchPrintfW(szName, MAX_PATH, _T("Style%d"), i);

		TCHAR szTemp[MAX_PATH];
		StringCchPrintfW(szTemp, MAX_PATH, _T("%08X|%08X"),
			theStyle.colFront, theStyle.colBak);

		SHSetValue(HKEY_CURRENT_USER, szTraceStyle, szName, 
			REG_SZ, szTemp, (DWORD)_tcslen(szTemp)*sizeof(TCHAR));
	}
	
}

//--------------------------------------------------------------------------------------------
void Config::addNewStyle(int styleID)
{
	assert(styleID>0 && styleID<MAX_TRACE_STYLE_COUNTS);
	if(styleID<=0 || styleID>=MAX_TRACE_STYLE_COUNTS) return;

	m_allTraceStyle[styleID].useDefault=false;
	m_allTraceStyle[styleID].colBak = m_allTraceStyle[0].colBak;
	m_allTraceStyle[styleID].colFront = m_allTraceStyle[0].colFront;
}

//--------------------------------------------------------------------------------------------
void Config::removeStyle(int styleID)
{
	assert(styleID>0 && styleID<MAX_TRACE_STYLE_COUNTS);
	if(styleID<=0 || styleID>=MAX_TRACE_STYLE_COUNTS) return;

	m_allTraceStyle[styleID].useDefault=true;
}

//--------------------------------------------------------------------------------------------
COLORREF Config::getForegroundColor(int styleID) const
{
	if(styleID<0 || styleID>=MAX_TRACE_STYLE_COUNTS || m_allTraceStyle[styleID].useDefault) 
		return m_allTraceStyle[0].colFront;

	return m_allTraceStyle[styleID].colFront;
}

//--------------------------------------------------------------------------------------------
void Config::setForegroundColor(int styleID, COLORREF col)
{
	assert(styleID>=0 && styleID<MAX_TRACE_STYLE_COUNTS);
	if(styleID<0 || styleID>=MAX_TRACE_STYLE_COUNTS) return;

	m_allTraceStyle[styleID].useDefault = false;
	m_allTraceStyle[styleID].colFront = col;
}

//--------------------------------------------------------------------------------------------
COLORREF Config::getBackgroundColor(int styleID) const
{
	if(styleID<0 || styleID>=MAX_TRACE_STYLE_COUNTS || m_allTraceStyle[styleID].useDefault) 
		return m_allTraceStyle[0].colBak;

	return m_allTraceStyle[styleID].colBak;
}

//--------------------------------------------------------------------------------------------
void Config::setBackgroundColor(int styleID, COLORREF col)
{
	assert(styleID>=0 && styleID<MAX_TRACE_STYLE_COUNTS);
	if(styleID<0 || styleID>=MAX_TRACE_STYLE_COUNTS) return;

	m_allTraceStyle[styleID].useDefault = false;
	m_allTraceStyle[styleID].colBak = col;
}

//--------------------------------------------------------------------------------------------
void Config::setFont(LPLOGFONT lf)
{
	if(m_hFont) ::DeleteObject(m_hFont);
	m_hFont = CreateFontIndirect(lf);
}

}