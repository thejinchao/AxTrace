#include "StdAfx.h"
#include "AT_Message.h"
#include "AT_Util.h"
#include "utf/ConvertUTF.h"

namespace AT3
{

//////////////////////////////////////////////////////////////////////////////////////////////
//Base Message
//////////////////////////////////////////////////////////////////////////////////////////////
Message::Message(void)
	: m_nProcessID(0)
	, m_nThreadID(0)
	, m_nWindowID(0)
	, m_nStyleID(0)
{
}

//--------------------------------------------------------------------------------------------
Message::~Message(void)
{
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
	m_nWindowID = 0;
	m_nStyleID = 0;

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
size_t _getValueLength(AXTRACE_VALUE_TYPE valueType)
{
	switch(valueType)
	{
	case AX_INT8: case AX_UINT8: return 1;
	case AX_INT16: case AX_UINT16: return 2;
	case AX_INT32: case AX_UINT32: return 4;
	case AX_INT64: case AX_UINT64: return 8;
	case AX_FLOAT32: return 4;
	case AX_FLOAT64: return 8;
	default: break;
	}
	return 0;
}

//--------------------------------------------------------------------------------------------
void ValueMessage::build(const AXIATRACE_TIME& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	memcpy(&m_traceTime, &traceTime, sizeof(m_traceTime));

	m_nProcessID = head.pid;
	m_nThreadID = head.tid;
	m_nWindowID = 0;
	m_nStyleID = 0;

	axtrace_value_s value_head;
	size_t len = ringBuf->memcpy_out(&value_head, sizeof(value_head));
	assert(len == sizeof(value_head));

	m_valuetype = value_head.value_type;
	m_valueSize = value_head.value_len;

	//copy name 
	char tempName[AXTRACE_MAX_VALUENAME_LENGTH];
	int name_length = value_head.name_len;
	len = ringBuf->memcpy_out(tempName, name_length+1);
	assert(len == name_length+1);
	wcscpy_s(m_name, AXTRACE_MAX_VALUENAME_LENGTH, convertUTF8ToUTF16(tempName, name_length+1));

	//value
	if ((m_valuetype == AX_STR_ACP || m_valuetype == AX_STR_UTF8 || m_valuetype == AX_STR_UTF16) && m_valueSize>STANDARD_VALUE_SIZE)
	{
		//string value
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
}

//--------------------------------------------------------------------------------------------
void ValueMessage::getValueAsString(std::wstring& value) const
{
	const int TEMP_STR_SIZE = 512;
	wchar_t temp[TEMP_STR_SIZE]={0};

	switch(m_valuetype)
	{
	case AX_INT8:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%d", *((__int8*)m_valueBuf)); break;
	case AX_UINT8:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%u", *((unsigned __int8*)m_valueBuf)); break;
	case AX_INT16:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%d", *((__int16*)m_valueBuf)); break;
	case AX_UINT16:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%u", *((unsigned __int16*)m_valueBuf));	break;
	case AX_INT32:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%d", *((__int32*)m_valueBuf));	break;
	case AX_UINT32:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%u", *((unsigned __int32*)m_valueBuf));	break;
	case AX_FLOAT32:
		{
			float abs_value = abs(*((float*)m_valueBuf));
			bool need_scientific_notation = ((abs_value>(1e+16)) || ((abs_value>0.f) && abs_value<(1e-16)));

			StringCchPrintfW(temp, TEMP_STR_SIZE, need_scientific_notation ? L"%e" : L"%.8f", *((float*)m_valueBuf));
		}
		break;
	case AX_INT64:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%I64d", *((__int64*)m_valueBuf));	break;
	case AX_UINT64:
		StringCchPrintfW(temp, TEMP_STR_SIZE, L"%I64u", *((unsigned __int64*)m_valueBuf));	break;
	case AX_FLOAT64:
		{
			double abs_value = abs(*((double*)m_valueBuf));
			bool need_scientific_notation = ((abs_value>(1e+16)) || (abs_value>0.0 && (abs_value<(1e-16))));

			StringCchPrintfW(temp, TEMP_STR_SIZE, need_scientific_notation ? L"%e" : L"%.16f", *((double*)m_valueBuf));
		}
		break;
	case AX_STR_ACP:
		{
			wchar_t* wszBuf = new wchar_t[m_valueSize];
			::MultiByteToWideChar(CP_ACP, 0, (const char*)m_valueBuf, m_valueSize, wszBuf, m_valueSize);
			value = wszBuf;
			delete[] wszBuf;
		}
		return;
	case AX_STR_UTF8:
		value = convertUTF8ToUTF16((const char*)m_valueBuf, m_valueSize); return;
	case AX_STR_UTF16:
		value = (const wchar_t*)m_valueBuf; return;
	}

	value = temp;
}

}
