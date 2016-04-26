#include "StdAfx.h"
#include "AT_Message.h"
#include "AT_Util.h"
#include "utf/ConvertUTF.h"

namespace AT3
{

const char* Message::MESSAGE_META_NAME = "AxTrace.Message";

//////////////////////////////////////////////////////////////////////////////////////////////
//Base Message
//////////////////////////////////////////////////////////////////////////////////////////////
Message::Message(void)
	: m_nProcessID(0)
	, m_nThreadID(0)
	, m_nStyleID(0)
{
}

//--------------------------------------------------------------------------------------------
Message::~Message(void)
{
}

//-------------------------------------------------------------------------------------
int _lua_get_type(lua_State *L)
{
	const Message* msg = (const Message*)lua_touserdata(L, 1);
	lua_pushinteger(L, msg->getTraceType());
	return 1;
}

//-------------------------------------------------------------------------------------
int _lua_get_process_id(lua_State *L)
{
	const Message* msg = (const Message*)lua_touserdata(L, 1);
	lua_pushinteger(L, msg->getProcessID());
	return 1;
}

//-------------------------------------------------------------------------------------
int _lua_get_thread_id(lua_State *L)
{
	const Message* msg = (const Message*)lua_touserdata(L, 1);
	lua_pushinteger(L, msg->getThreadID());
	return 1;
}

//-------------------------------------------------------------------------------------
int _lua_get_style(lua_State *L)
{
	const Message* msg = (const Message*)lua_touserdata(L, 1);
	lua_pushinteger(L, msg->getStyleID());
	return 1;
}

//-------------------------------------------------------------------------------------
int _lua_get_content(lua_State *L)
{
	const Message* msg = (const Message*)lua_touserdata(L, 1);

	if (msg->getTraceType() == AXTRACE_CMD_TYPE_TRACE) {
		const wchar_t* msg_content = ((LogMessage*)msg)->getLogBuf();
		size_t msg_char_length = ((LogMessage*)msg)->getLogSizeChar() + 1;

		lua_pushstring(L, convertUTF16ToUTF8(msg_content, msg_char_length));

		return 1;
	}
	else if (msg->getTraceType() == AXTRACE_CMD_TYPE_VALUE)
	{
		std::wstring value_as_string;

		((ValueMessage*)msg)->getValueAsString(value_as_string);

		lua_pushstring(L, convertUTF16ToUTF8(value_as_string.c_str(), value_as_string.length()+1));
		return 1;
	}
	return 0;
}

//--------------------------------------------------------------------------------------------
void Message::_luaopen(lua_State *L)
{
	static luaL_Reg msg_data_meta[] =
	{
		{ "get_type", _lua_get_type },
		{ "get_process_id", _lua_get_process_id },
		{ "get_thread_id", _lua_get_thread_id },
		{ "get_style", _lua_get_style },
		{ "get_content", _lua_get_content },
		{ 0, 0 }
	};


	//PlayerData meta table
	luaL_newmetatable(L, MESSAGE_META_NAME);
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */

	luaL_register(L, NULL, msg_data_meta);  /* file methods */
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Log Message
//////////////////////////////////////////////////////////////////////////////////////////////
LogMessage::LogMessage(void)
	: m_pLogBuf(0)
	, m_pLogBufInChar(0)
	, Message()
{
}

//--------------------------------------------------------------------------------------------
LogMessage::~LogMessage(void)
{
	if(m_pLogBuf) 
	{
		delete[] m_pLogBuf; m_pLogBuf=0;
	}
}


//--------------------------------------------------------------------------------------------
void LogMessage::build(const AXIATRACE_TIME& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	void* rc = 0;
	memcpy(&m_traceTime, &traceTime, sizeof(m_traceTime));

	m_nProcessID = head.pid;
	m_nThreadID = head.tid;
	m_nStyleID = head.style;

	axtrace_trace_s trace_head;
	size_t len = ringBuf->memcpy_out(&trace_head, sizeof(trace_head));
	assert(len == sizeof(trace_head));

	//get codepage
	int codePage = trace_head.code_page;

	//get log length
	int logLength = trace_head.length;

	//get log
	if(m_pLogBuf) delete[] m_pLogBuf; 

	if(codePage == ATC_UTF16)
	{
		m_pLogBufInChar = logLength/2;
		m_pLogBuf=new wchar_t[m_pLogBufInChar+1];

		size_t len = ringBuf->memcpy_out(m_pLogBuf, logLength);
		assert(len == logLength);

		m_pLogBuf[m_pLogBufInChar]=0;
	}
	else if(codePage == ATC_UTF8)
	{
		char* tempBuf = new char[logLength];
		size_t len = ringBuf->memcpy_out(tempBuf, logLength);
		assert(len == logLength);

		//convert to utf16
		m_pLogBufInChar = logLength;
		m_pLogBuf=new wchar_t[m_pLogBufInChar+1];

		const UTF8* sourceStart = (const UTF8*)tempBuf;
		UTF16* targetStart = (UTF16*)m_pLogBuf;

		::ConvertUTF8toUTF16(&sourceStart, sourceStart+logLength, 
			&targetStart, (UTF16*)(m_pLogBuf+m_pLogBufInChar+1), strictConversion);

		delete[] tempBuf; tempBuf=0;

		*targetStart = 0;
		m_pLogBufInChar = wcslen(m_pLogBuf);

	}
	else if(codePage == ATC_ACP)
	{
		char* tempBuf = new char[logLength+1];
		size_t len = ringBuf->memcpy_out(tempBuf, logLength);
		assert(len == logLength);

		tempBuf[logLength] = 0;

		m_pLogBufInChar = logLength;
		m_pLogBuf=new wchar_t[m_pLogBufInChar+1];
		::MultiByteToWideChar(CP_ACP, 0, tempBuf, logLength+1, m_pLogBuf, m_pLogBufInChar+1);

		delete[] tempBuf; tempBuf=0;

		m_pLogBufInChar = wcslen(m_pLogBuf);
	}
	else
	{
		//error!
		assert(false);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
//Value Message
//////////////////////////////////////////////////////////////////////////////////////////////
ValueMessage::ValueMessage(void)
	: Message()
	, m_valueBuf(0)
	, m_valueSize(0)
{
	memset(m_name, 0, AXTRACE_MAX_VALUENAME_LENGTH*sizeof(wchar_t));
}

//--------------------------------------------------------------------------------------------
ValueMessage::~ValueMessage(void)
{
	if(m_valueBuf && m_valueBuf!=m_standValueBuf)
	{
		delete[] m_valueBuf;
	}
}

//--------------------------------------------------------------------------------------------
void ValueMessage::build(const AXIATRACE_TIME& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	memcpy(&m_traceTime, &traceTime, sizeof(m_traceTime));

	m_nProcessID = head.pid;
	m_nThreadID = head.tid;
	m_nStyleID = head.style;

	axtrace_value_s value_head;
	size_t len = ringBuf->memcpy_out(&value_head, sizeof(value_head));
	assert(len == sizeof(value_head));

	m_valuetype = value_head.value_type;
	m_valueSize = value_head.value_len;

	//copy name 
	char tempName[AXTRACE_MAX_VALUENAME_LENGTH];
	int name_length = value_head.name_len;
	//TODO: check name length
	len = ringBuf->memcpy_out(tempName, name_length);
	assert(len == name_length);
	tempName[name_length-1] = 0; //make sure last char is '\0'
	wcscpy_s(m_name, AXTRACE_MAX_VALUENAME_LENGTH, convertUTF8ToUTF16(tempName, name_length+1));

	//value
	if (m_valueSize>STANDARD_VALUE_SIZE)
	{
		//big value
		m_valueBuf = new char[m_valueSize];
		memset(m_valueBuf, 0, m_valueSize);
	}
	else
	{
		m_valueBuf = m_standValueBuf;
	}

	//value
	len = ringBuf->memcpy_out(m_valueBuf, m_valueSize);
	assert(len == m_valueSize);

	//make sure '\0' ended
	if (m_valuetype == AXV_STR_ACP || m_valuetype == AXV_STR_UTF8)
	{
		((char*)m_valueBuf)[m_valueSize-1] = 0;
	}
	else if (m_valuetype == AXV_STR_UTF16)
	{
		((char*)m_valueBuf)[m_valueSize-1] = 0;
		((char*)m_valueBuf)[m_valueSize-2] = 0;
	}
}

//--------------------------------------------------------------------------------------------
void ValueMessage::getValueAsString(std::wstring& value) const
{
	const int TEMP_STR_SIZE = 512;
	wchar_t temp[TEMP_STR_SIZE]={0};

	switch(m_valuetype)
	{
	case AXV_INT8:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%d", *((__int8*)m_valueBuf)); break;
	case AXV_UINT8:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%u", *((unsigned __int8*)m_valueBuf)); break;
	case AXV_INT16:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%d", *((__int16*)m_valueBuf)); break;
	case AXV_UINT16:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%u", *((unsigned __int16*)m_valueBuf));	break;
	case AXV_INT32:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%d", *((__int32*)m_valueBuf));	break;
	case AXV_UINT32:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%u", *((unsigned __int32*)m_valueBuf));	break;
	case AXV_FLOAT32:
		{
			float abs_value = abs(*((float*)m_valueBuf));
			bool need_scientific_notation = ((abs_value>(1e+16)) || ((abs_value>0.f) && abs_value<(1e-16)));

			StringCchPrintfW(temp, TEMP_STR_SIZE, need_scientific_notation ? L"%e" : L"%.8f", *((float*)m_valueBuf));
		}
		break;
	case AXV_INT64:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%I64d", *((__int64*)m_valueBuf));	break;
	case AXV_UINT64:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%I64u", *((unsigned __int64*)m_valueBuf));	break;
	case AXV_FLOAT64:
		{
			double abs_value = abs(*((double*)m_valueBuf));
			bool need_scientific_notation = ((abs_value>(1e+16)) || (abs_value>0.0 && (abs_value<(1e-16))));

			StringCchPrintfW(temp, TEMP_STR_SIZE, need_scientific_notation ? L"%e" : L"%.16f", *((double*)m_valueBuf));
		}
		break;
	case AXV_STR_ACP:
		{
			wchar_t* wszBuf = new wchar_t[m_valueSize];
			::MultiByteToWideChar(CP_ACP, 0, (const char*)m_valueBuf, m_valueSize, wszBuf, m_valueSize);
			value = wszBuf;
			delete[] wszBuf;
		}
		return;
	case AXV_STR_UTF8:
		value = convertUTF8ToUTF16((const char*)m_valueBuf, m_valueSize); return;
	case AXV_STR_UTF16:
		value = (const wchar_t*)m_valueBuf; return;
	}

	value = temp;
}

}
