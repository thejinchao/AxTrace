/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#include "StdAfx.h"
#include "AT_Config.h"
#include "AT_Util.h"
#include "AT_Filter.h"

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

	if(m_hFont) ::DeleteObject(m_hFont);

	LOGFONT lf={0};
	::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
	m_hFont = CreateFontIndirect(&lf);

	//set default
	COLORREF colBak = GetSysColor(COLOR_WINDOW);
	COLORREF colFront = GetSysColor(COLOR_WINDOWTEXT);

	m_filterScript = 
		"function onTraceMessage(msg) \n"
		" local frontColor=COL_BLACK; \n"
		" local backColor=COL_WHITE; \n"
		" local msgStyle=msg:get_style(); \n"
		" if(msgStyle>=AXT_ERROR) then \n"
		"   frontColor=COL_RED; \n"
		"   if(msgStyle==AXT_FATAL) then backColor=COL_YELLOW; end; \n"
		" end; \n"
		" return true, \"defult\", frontColor, backColor; \n"
		"end; \n"
		""
		"function onValueMessage(msg) \n"
		" return true, \"defult\", COL_BLACK, COL_WHITE; \n"
		"end; \n"
		;
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

	m_filterScript = other.m_filterScript;
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

	// Load filter script
	wchar_t* szScriptBuf = new wchar_t[Filter::MAX_SCRIPT_LENGTH_IN_CHAR];
	size = Filter::MAX_SCRIPT_LENGTH_IN_CHAR * sizeof(wchar_t);
	if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, g_szAxTrace3Key, _T("FilterScript"), &type, szScriptBuf, &size))
	{
		m_filterScript = convertUTF16ToUTF8(szScriptBuf, size/ sizeof(wchar_t));
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

	// Filter script
	const wchar_t* wszScript = convertUTF8ToUTF16(m_filterScript.c_str(), m_filterScript.length()+1);
	SHSetValue(HKEY_CURRENT_USER, g_szAxTrace3Key, _T("FilterScript"),
		REG_SZ, wszScript, (DWORD)_tcslen(wszScript) * sizeof(wchar_t));
}

//--------------------------------------------------------------------------------------------
void Config::setFont(LPLOGFONT lf)
{
	if(m_hFont) ::DeleteObject(m_hFont);
	m_hFont = CreateFontIndirect(lf);
}

}