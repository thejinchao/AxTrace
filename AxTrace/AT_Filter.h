#pragma once

namespace AT3
{

//predefine 
class LogMessage;
class ValueMessage;
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
	void init(void);
	bool reloadScript(const Config* cfg, HWND hwnd);

	void onTraceMessage(const LogMessage* message, Result& result);
	void onValueMessage(const ValueMessage* message, Result& result);

public:
	static void _luaopen(lua_State*	L);

private:
	lua_State* L;

public:
	Filter();
	virtual ~Filter();
};

}
