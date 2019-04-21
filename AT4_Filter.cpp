/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_Filter.h"
#include "AT4_Config.h"
#include "AT4_Message.h"

//--------------------------------------------------------------------------------------------
Filter::Filter()
	: m_config(nullptr)
	, L(nullptr)
{

}

//--------------------------------------------------------------------------------------------
Filter::~Filter()
{
	lua_close(L);
}

//--------------------------------------------------------------------------------------------
bool Filter::init(Config* cfg)
{
	assert(cfg);

	m_config = cfg;

	L = luaL_newstate();
	luaL_openlibs(L);

	//register functions
	_luaopen(L);
	LogMessage::_luaopen(L);
	ValueMessage::_luaopen(L);
	Begin2DSceneMessage::_luaopen(L);
	Update2DActorMessage::_luaopen(L);

	//reload script
	if (!reloadScript(m_config->getFilterScript().toUtf8().toStdString().c_str()))
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------
bool Filter::reloadScript(const char* script)
{
	//run init lua script
	if (luaL_dostring(L, script)) {
		const char* error_msg = lua_tostring(L, -1);
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("LoadScript Error"));
		msgBox.setText(QString::fromUtf8(error_msg));
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.exec();
		return false;
	}
	return true;
}

//--------------------------------------------------------------------------------------------
bool Filter::tryLoadScript(const char* script, QString& errorMsg)
{
	lua_State* L2 = luaL_newstate();
	luaL_openlibs(L2);

	//register functions
	_luaopen(L2);
	LogMessage::_luaopen(L2);
	ValueMessage::_luaopen(L2);
	Begin2DSceneMessage::_luaopen(L2);
	Update2DActorMessage::_luaopen(L2);

	bool success = (luaL_dostring(L2, script) == 0);

	errorMsg = QString::fromUtf8(lua_tostring(L2, -1));
	lua_close(L2);

	return success;
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

	lua_pushinteger(L, 0); 	lua_setglobal(L, "ACTOR_CIRCLE");
	lua_pushinteger(L, 1); 	lua_setglobal(L, "ACTOR_QUAD");
}

//--------------------------------------------------------------------------------------------
void Filter::onLogMessage(const LogMessage* message, ListResult& result)
{
	lua_getglobal(L, "onLogMessage");

	lua_pushlightuserdata(L, (void*)message);

	luaL_getmetatable(L, LogMessage::MetaName);
	lua_setmetatable(L, -2);
	lua_pcall(L, 1, 4, 0);

	result.display = (lua_toboolean(L, -4) != 0);
	if (result.display) {
		result.wndTitle = lua_tostring(L, -3);
		result.fontColor = (uint16_t)(lua_tointeger(L, -2) & 0xFFF);
		result.backColor = (uint16_t)(lua_tointeger(L, -1) & 0xFFF);
	}

	lua_pop(L, 4);
}

//--------------------------------------------------------------------------------------------
void Filter::onValueMessage(const ValueMessage* message, ListResult& result)
{
	lua_getglobal(L, "onValueMessage");
	lua_pushlightuserdata(L, (void*)message);

	luaL_getmetatable(L, ValueMessage::MetaName);
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
void Filter::onActor2DMessage(const Update2DActorMessage* message, Actor2DResult& result)
{
	lua_getglobal(L, "onActor2DMessage");
	lua_pushlightuserdata(L, (void*)message);

	luaL_getmetatable(L, Update2DActorMessage::MetaName);
	lua_setmetatable(L, -2);
	lua_pcall(L, 1, 5, 0);

	result.display = (lua_toboolean(L, -5) != 0);
	if (result.display) {
		result.type = (Actor2DType)lua_tointeger(L, -4);
		result.size = lua_tointeger(L, -3);
		result.borderColor = (uint16_t)(lua_tointeger(L, -2) & 0xFFFF);
		result.fillColor = (uint16_t)(lua_tointeger(L, -1) & 0xFFFF);
	}
	lua_pop(L, 5);
}
