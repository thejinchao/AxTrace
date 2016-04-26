/***************************************************

				 	AXIA|Trace3

							(C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_Filter.h"

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

	//run init lua script
	static const char* init_script =
		"function onTraceMessage(msg) \n"
		" local frontColor=BLACK; \n"
		" local backColor=WHITE; \n"
		" local msgStyle=msg:get_style(); \n"
		" if(msgStyle>=4) then \n"
		"   frontColor=RED; \n"
		"   if(msgStyle==5) then backColor=YELLOW; end; \n"
		" end; \n"
		" return true, \"defult\", frontColor, backColor; \n" 
		"end; \n" 
		"" 
		"function onValueMessage(msg) \n" 
		" return true, \"defult\", BLACK, WHITE; \n" 
		"end; \n"
		;
	if (luaL_dostring(L, init_script)) {
		const char* error_msg = lua_tostring(L, -1);
		return;
	}

}

//--------------------------------------------------------------------------------------------
void Filter::_luaopen(lua_State* L)
{
	lua_pushinteger(L, 0x000); 	lua_setglobal(L, "BLACK");
	lua_pushinteger(L, 0xFFF); 	lua_setglobal(L, "WHITE");
	lua_pushinteger(L, 0x00F); 	lua_setglobal(L, "RED");
	lua_pushinteger(L, 0x0F0); 	lua_setglobal(L, "GREEN");
	lua_pushinteger(L, 0xF00); 	lua_setglobal(L, "BLUE");
	lua_pushinteger(L, 0x777); 	lua_setglobal(L, "GRAY");
	lua_pushinteger(L, 0x0FF); 	lua_setglobal(L, "YELLOW");

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
