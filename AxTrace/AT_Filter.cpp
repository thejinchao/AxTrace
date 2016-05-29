/***************************************************

				 	AXIA|Trace3

							(C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_Filter.h"
#include "AT_Config.h"
#include "AT_System.h"
#include "AT_MainFrame.h"
#include "AT_Message.h"
#include "AT_Util.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
Filter::Filter()
	: L(0)
	, m_cfg(0)
	, m_hNotepadProcess(0)
	, m_hMonitorThread(0)
{
}

//--------------------------------------------------------------------------------------------
Filter::~Filter()
{
}

//--------------------------------------------------------------------------------------------
void Filter::init(Config* cfg)
{
	assert(cfg);

	L = luaL_newstate();
	luaL_openlibs(L);

	//register functions
	Filter::_luaopen(L);
	Message::_luaopen(L);

	m_cfg = cfg;
}

//--------------------------------------------------------------------------------------------
bool Filter::reloadScript(const char* script, HWND hwnd)
{
	//run init lua script
	if (luaL_dostring(L, script)) {
		const char* error_msg = lua_tostring(L, -1);
		MessageBoxA(hwnd, error_msg, "LoadScript Error", MB_OK | MB_ICONSTOP);
		return false;
	}
	return true;
}

//--------------------------------------------------------------------------------------------
void Filter::_luaopen(lua_State* L)
{
	lua_pushinteger(L, 0x000); 	lua_setglobal(L, "COL_BLACK");
	lua_pushinteger(L, 0xFFF); 	lua_setglobal(L, "COL_WHITE");
	lua_pushinteger(L, 0x00F); 	lua_setglobal(L, "COL_RED");
	lua_pushinteger(L, 0x0F0); 	lua_setglobal(L, "COL_GREEN");
	lua_pushinteger(L, 0xF00); 	lua_setglobal(L, "COL_BLUE");
	lua_pushinteger(L, 0x777); 	lua_setglobal(L, "COL_GRAY");
	lua_pushinteger(L, 0x0FF); 	lua_setglobal(L, "COL_YELLOW");
	lua_pushinteger(L, 0x06F); 	lua_setglobal(L, "COL_ORANGE");
	lua_pushinteger(L, 0xF0F); 	lua_setglobal(L, "COL_VIOLET");

	lua_pushinteger(L, 0); 	lua_setglobal(L, "AXT_TRACE");
	lua_pushinteger(L, 1); 	lua_setglobal(L, "AXT_DEBUG");
	lua_pushinteger(L, 2); 	lua_setglobal(L, "AXT_INFO");
	lua_pushinteger(L, 3); 	lua_setglobal(L, "AXT_WARN");
	lua_pushinteger(L, 4); 	lua_setglobal(L, "AXT_ERROR");
	lua_pushinteger(L, 5); 	lua_setglobal(L, "AXT_FATAL");
	lua_pushinteger(L, 10); lua_setglobal(L, "AXT_USERDEF");
}

//--------------------------------------------------------------------------------------------
void Filter::onLogMessage(const LogMessage* message, Result& result)
{
	lua_getglobal(L, "onLogMessage");

	lua_pushlightuserdata(L, (void*)message);

	luaL_getmetatable(L, Message::MESSAGE_META_NAME);
	lua_setmetatable(L, -2);
	lua_pcall(L, 1, 4, 0);

	result.display = (lua_toboolean(L, -4)!=0);
	if (result.display) {
		result.wndTitle = lua_tostring(L, -3);
		result.fontColor = (uint16_t)(lua_tointeger(L, -2) & 0xFFF);
		result.backColor = (uint16_t)(lua_tointeger(L, -1) & 0xFFF);
	}

	lua_pop(L, 4);
}

//--------------------------------------------------------------------------------------------
void Filter::onValueMessage(const ValueMessage* message, Result& result)
{
	lua_getglobal(L, "onValueMessage");
	lua_pushlightuserdata(L, (void*)message);

	luaL_getmetatable(L, Message::MESSAGE_META_NAME);
	lua_setmetatable(L, -2);
	lua_pcall(L, 1, 4, 0);

	result.display = (lua_toboolean(L, -4) != 0);
	if (result.display) {
		result.wndTitle = lua_tostring(L, -3);
		result.fontColor = (uint16_t)(lua_tointeger(L, -2) & 0xFFFF);
		result.backColor = (uint16_t)(lua_tointeger(L, -1) & 0xFFFF);
	}
	lua_pop(L, 4);
}

//--------------------------------------------------------------------------------------------
static BOOL CALLBACK _findCurrentNotepad(HWND hWnd, LPARAM lParam)
{
	DWORD notepadProcessID = (DWORD)lParam;
	
	DWORD processID=0;
	::GetWindowThreadProcessId(hWnd, &processID);

	if (processID == notepadProcessID) {
		::SetForegroundWindow(hWnd);
		return false;
	}
	return true;
}

//--------------------------------------------------------------------------------------------
bool Filter::editScriptWithNotepad(void)
{
	if (m_hNotepadProcess != 0 && ::WaitForSingleObject(m_hNotepadProcess, 0)!=WAIT_OBJECT_0) {
		//set current notepad window to foreground
		EnumWindows(_findCurrentNotepad, (LPARAM)m_dwNotepadProcessID);
		return true;
	}

	//get temp path
	wchar_t wszTempPath[MAX_PATH] = { 0 };
	GetTempPath(MAX_PATH, wszTempPath);
	PathAppend(wszTempPath, L"axtrace_temp");
	if (!CreateDirectory(wszTempPath, 0) && ::GetLastError() != ERROR_ALREADY_EXISTS) {
		return false;
	}
	m_strTempPath = wszTempPath;

	//create temp file
	wchar_t wszTempFile[MAX_PATH] = { 0 };
	StringCchCopy(wszTempFile, MAX_PATH, wszTempPath);
	PathAppend(wszTempFile, L"script.lua");
	HANDLE hTempFile = ::CreateFile(wszTempFile,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_ARCHIVE,
		0);

	if (hTempFile == INVALID_HANDLE_VALUE) {
		return false;
	}
	m_strTempFile = wszTempFile;

	//write script to file
	DWORD dwBytesWrite;
	if (!::WriteFile(hTempFile, m_cfg->getFilterScript().c_str(), m_cfg->getFilterScript().length(), &dwBytesWrite, 0) ||
		dwBytesWrite != m_cfg->getFilterScript().length())
	{
		CloseHandle(hTempFile);
		return false;
	}
	CloseHandle(hTempFile);

	//get file attribute
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	if (!GetFileAttributesEx(wszTempFile, GetFileExInfoStandard, &wfad)) {
		return false;
	}
	m_timeScriptLastWriteTime = wfad.ftLastWriteTime;

	//command line
	wchar_t wszNotepad[MAX_PATH] = { 0 };
	::GetSystemDirectory(wszNotepad, MAX_PATH);
	::PathAppend(wszNotepad, L"notepad.exe");

	wchar_t wszCommandLine[MAX_PATH] = { 0 };
	StringCchPrintf(wszCommandLine, MAX_PATH, L"%s \"%s\"", wszNotepad, wszTempFile);

	//create a notepad process to edit
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;

	if (CreateProcess(NULL, wszCommandLine, NULL, NULL,
		FALSE, CREATE_DEFAULT_ERROR_MODE, NULL,
		wszTempPath, &si, &pi) == FALSE) {
		return false;
	}
	m_hNotepadProcess = pi.hProcess;
	m_dwNotepadProcessID = pi.dwProcessId;

	//create monitor thread
	unsigned int thread_id;
	m_hMonitorThread = (HANDLE)::_beginthreadex(0, 0,
		_monitor_thread_entry, (void*)(this),
		THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME, &thread_id);


	return true;
}

//--------------------------------------------------------------------------------------------
unsigned int Filter::_monitor_thread_entry(void)
{
	HANDLE hChangeEvent = FindFirstChangeNotification(m_strTempPath.c_str(), FALSE, 
		FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_ATTRIBUTES);
	if (hChangeEvent == INVALID_HANDLE_VALUE) {
		return 1;
	}

	for (;;) {
		HANDLE hWaitEvent[2];
		hWaitEvent[0] = m_hNotepadProcess;
		hWaitEvent[1] = hChangeEvent;

		DWORD result = ::WaitForMultipleObjects(2, hWaitEvent, FALSE, INFINITE);

		if (result == WAIT_OBJECT_0) {
			//notepad process be closed
			break;
		}
		else if (result == WAIT_OBJECT_0 + 1) {
			//is script file be modified
			WIN32_FILE_ATTRIBUTE_DATA wfad;
			if (!GetFileAttributesEx(m_strTempFile.c_str(), GetFileExInfoStandard, &wfad)) {
				return false;
			}
			if (wfad.ftLastWriteTime.dwHighDateTime != m_timeScriptLastWriteTime.dwHighDateTime || 
				wfad.ftLastWriteTime.dwLowDateTime!= m_timeScriptLastWriteTime.dwLowDateTime) {
				//notify main thread to reload script
				System::getSingleton()->getMainFrame()->SendMessageW(MainFrame::WM_ON_RELOADSCRIPT_MESSAGE, (WPARAM)m_strTempFile.c_str(), 0);

				m_timeScriptLastWriteTime = wfad.ftLastWriteTime;
			}

			//wait next change
			if (FALSE == FindNextChangeNotification(hChangeEvent)) {
				break;
			}
			continue;
		}

	}

	//Close change event
	FindCloseChangeNotification(hChangeEvent);

	//close notepad process;
	CloseHandle(m_hNotepadProcess);
	m_hNotepadProcess = 0;

	CloseHandle(m_hMonitorThread);
	m_hMonitorThread = 0;

	return 0;
}

//--------------------------------------------------------------------------------------------
bool Filter::tryReloadScriptFile(const char* script, std::string& errorMessage)
{
	lua_State* testL = luaL_newstate();
	luaL_openlibs(testL);

	Filter::_luaopen(testL);
	Message::_luaopen(testL);

	//run init lua script
	if (luaL_dostring(testL, script)) {
		errorMessage = lua_tostring(testL, -1);
		lua_close(testL);
		return false;
	}

	//check necessary function
	lua_getglobal(testL, "onLogMessage");
	if (!lua_isfunction(testL, -1)) {
		errorMessage = "Missing function \"onLogMessage\"";
		lua_close(testL);
		return false;
	}
	lua_pop(testL, 1);

	lua_getglobal(testL, "onValueMessage");
	if (!lua_isfunction(testL, -1)) {
		errorMessage = "Missing function \"onValueMessage\"";
		lua_close(testL);
		return false;
	}
	lua_pop(testL, 1);

	lua_close(testL);
	return true;
}

}
