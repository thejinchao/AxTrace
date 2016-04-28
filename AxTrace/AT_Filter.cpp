/***************************************************

				 	AXIA|Trace3

							(C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_Filter.h"
#include "AT_Config.h"

#include "AT_Message.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
Filter::Filter()
{
}

//--------------------------------------------------------------------------------------------
Filter::~Filter()
{
}

//--------------------------------------------------------------------------------------------
void Filter::init(void)
{
	L = luaL_newstate();
	luaL_openlibs(L);

	//register functions
	Filter::_luaopen(L);
	Message::_luaopen(L);
}

//--------------------------------------------------------------------------------------------
bool Filter::reloadScript(const Config* cfg, HWND hwnd)
{
	//run init lua script
	if (luaL_dostring(L, cfg->getFilterScript().c_str())) {
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
void Filter::onTraceMessage(const LogMessage* message, Result& result)
{
	lua_getglobal(L, "onTraceMessage");

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

}
