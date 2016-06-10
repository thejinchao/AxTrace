#pragma once

namespace AT3
{

//predefine 
class LogMessage;
class ValueMessage;
class G2DCleanMapMessage;
class G2DActorMessage;
class Config;

class Filter
{
public:

	struct Result
	{
		bool		display;
		std::string wndTitle;		//utf8
		uint16_t	fontColor;
		uint16_t	backColor;
	};

	enum { MAX_SCRIPT_LENGTH_IN_CHAR = 1024*1024*2 };

public:
	void init(Config* cfg);
	bool reloadScript(const char* script, HWND hwnd);
	bool tryReloadScriptFile(const char* script, std::string& errorMessage);

	void onLogMessage(const LogMessage* message, Result& result);
	void onValueMessage(const ValueMessage* message, Result& result);
	void on2DCleanMapMessage(const G2DCleanMapMessage* message, Result& result);
	void on2DActorMessage(const G2DActorMessage* message, Result& result);

	bool editScriptWithNotepad(void);

public:
	static void _luaopen(lua_State*	L);

private:
	lua_State* L;
	Config* m_cfg;
	std::wstring m_strTempPath;
	std::wstring m_strTempFile;
	HANDLE m_hNotepadProcess;
	HANDLE m_hMonitorThread;
	DWORD m_dwNotepadProcessID;
	FILETIME m_timeScriptLastWriteTime;

private:
	static unsigned int __stdcall _monitor_thread_entry(void* param) {
		return ((Filter*)param)->_monitor_thread_entry();
	}
	unsigned int _monitor_thread_entry(void);

public:
	Filter();
	virtual ~Filter();
};

}
