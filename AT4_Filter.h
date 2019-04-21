/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

class Config;
class LogMessage;
class ValueMessage;
class Update2DActorMessage;
struct lua_State;

class Filter
{
public:
	struct ListResult
	{
		bool		display;
		QString		wndTitle;		//utf8
		uint16_t	fontColor;
		uint16_t	backColor;
	};

	enum Actor2DType
	{
		AT_CIRCLE,
		AT_QUAD,
	};

	struct Actor2DResult
	{
		bool		display;
		Actor2DType	type;
		int			size;
		uint16_t	borderColor;
		uint16_t	fillColor;
	};

public:
	bool init(Config* cfg);

	bool reloadScript(const char* script);
	static bool tryLoadScript(const char* script, QString& errorMsg);

	void onLogMessage(const LogMessage* message, ListResult& result);
	void onValueMessage(const ValueMessage* message, ListResult& result);
	void onActor2DMessage(const Update2DActorMessage* msg, Actor2DResult& result);

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
	Filter();
	virtual ~Filter();
};