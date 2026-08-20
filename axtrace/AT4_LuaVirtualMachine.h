/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#pragma once

class Config;
class LogMessage;
class ValueMessage;
class Update2DActorMessage;
struct lua_State;
struct LogFilterResult;
struct ValueFilterResult;
struct Actor2DFilterResult;

class LuaVirtualMachine
{
public:
	bool init(Config* cfg);

	bool reloadScript(const char* script);
	static bool tryLoadScript(const char* script, QString& errorMsg);

	void onLogMessage(const LogMessage* message, LogFilterResult& result);
	void onValueMessage(const ValueMessage* message, ValueFilterResult& result);
	void onActor2DMessage(const Update2DActorMessage* msg, Actor2DFilterResult& result);

	static QColor toQColor(uint16_t col) {
		#define MAKE_DOUBLE(c)  (((c)&0xF)<<4|((c)&0xF))
		return QColor(MAKE_DOUBLE(col), MAKE_DOUBLE(col >> 4), MAKE_DOUBLE(col >> 8));
	}

private:
	static void _luaopen(lua_State* L);

private:
	Config*		m_config;
	lua_State*	L;

public:
	LuaVirtualMachine();
	virtual ~LuaVirtualMachine();
};