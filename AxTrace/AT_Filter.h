#pragma once

namespace AT3
{

//predefine 
class LogMessage;
class ValueMessage;

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

public:
	void init(void);

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
