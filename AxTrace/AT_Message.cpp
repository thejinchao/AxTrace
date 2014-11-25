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
void LogMessage::build(const AXIATRACE_TIME& traceTime, const AXIATRACE_DATAHEAD& head, ringbuf_t ringBuf)
{
	void* rc = 0;
	memcpy(&m_traceTime, &traceTime, sizeof(m_traceTime));

	m_nProcessID = head.dwProcessID;
	m_nWindowID = head.cWinID;
	m_nStyleID = head.cStyleID;

	//get codepage
	int codePage;
	rc = ringbuf_memcpy_from(&codePage, ringBuf, sizeof(int));
	assert(rc);

	//get log length
	int logLength;
	ringbuf_memcpy_from(&logLength, ringBuf, sizeof(int));
	assert(rc);

	//get log
	if(m_pLogBuf) delete[] m_pLogBuf; 

	if(codePage == ATC_UTF16)
	{
		m_pLogBufInChar = logLength/2;
		m_pLogBuf=new wchar_t[m_pLogBufInChar];

		rc = ringbuf_memcpy_from(m_pLogBuf, ringBuf, logLength);
		assert(rc!=0);

		m_pLogBuf[m_pLogBufInChar-1]=0;
	}
	else if(codePage == ATC_UTF8)
	{
		char* tempBuf = new char[logLength];
		rc = ringbuf_memcpy_from(tempBuf, ringBuf, logLength);
		assert(rc!=0);

		//convert to utf16
		m_pLogBufInChar = logLength;
		m_pLogBuf=new wchar_t[m_pLogBufInChar];

		const UTF8* sourceStart = (const UTF8*)tempBuf;
		UTF16* targetStart = (UTF16*)m_pLogBuf;

		::ConvertUTF8toUTF16(&sourceStart, sourceStart+logLength, 
			&targetStart, (UTF16*)(m_pLogBuf+m_pLogBufInChar), strictConversion);

		delete[] tempBuf; tempBuf=0;

		m_pLogBuf[m_pLogBufInChar-1]=0;
		m_pLogBufInChar = wcslen(m_pLogBuf);

	}
	else if(codePage == ATC_ACP)
	{
		char* tempBuf = new char[logLength];
		rc = ringbuf_memcpy_from(tempBuf, ringBuf, logLength);
		assert(rc!=0);

		m_pLogBufInChar = logLength;
		m_pLogBuf=new wchar_t[m_pLogBufInChar];
		::MultiByteToWideChar(CP_ACP, 0, tempBuf, logLength, m_pLogBuf, m_pLogBufInChar);

		delete[] tempBuf; tempBuf=0;

		m_pLogBuf[m_pLogBufInChar-1]=0;
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
	: m_valueBuf(0)
	, m_valueSize(0)
	, Message()
{
	memset(m_name, 0, AT_MaxValueNameLength*sizeof(wchar_t));
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
void ValueMessage::build(const AXIATRACE_TIME& traceTime, const AXIATRACE_DATAHEAD& head, ringbuf_t ringBuf)
{
	memcpy(&m_traceTime, &traceTime, sizeof(m_traceTime));

	m_nProcessID = head.dwProcessID;
	m_nWindowID = head.cWinID;
	m_nStyleID = head.cStyleID;

	int value_type;
	int name_length;

	void* rc = ringbuf_memcpy_from(&value_type, ringBuf, sizeof(int));
	assert(rc!=0 && value_type>=0 && value_type<AX_MAX_VALUETYPE);
	m_valuetype = (AXTRACE_VALUE_TYPE)value_type;

	rc = ringbuf_memcpy_from(&name_length, ringBuf, sizeof(int));
	assert(rc!=0 && name_length<AT_MaxValueNameLength);

	//copy name 
	char tempName[AT_MaxValueNameLength];
	rc = ringbuf_memcpy_from(tempName, ringBuf, name_length);	//TODO: length check!
	assert(rc!=0);
	wcscpy_s(m_name, AT_MaxValueNameLength, convertUTF8ToUTF16(tempName, name_length));

	//value length
	rc = ringbuf_memcpy_from(&m_valueSize, ringBuf, sizeof(m_valueSize));
	assert(rc!=0);

	//value
	if(m_valuetype==AX_STR_ACP || m_valuetype==AX_STR_UTF8 || m_valuetype==AX_STR_UTF16)
	{
		//string value
		m_valueBuf = new char[m_valueSize];
	
	}
	else
	{
		m_valueBuf = m_standValueBuf;
	}

	//value
	rc = ringbuf_memcpy_from(m_valueBuf, ringBuf, m_valueSize);
	assert(rc!=0);
}

//--------------------------------------------------------------------------------------------
void ValueMessage::getValueAsString(std::wstring& value) const
{
	const int TEMP_STR_SIZE = 64;
	wchar_t temp[TEMP_STR_SIZE]={0};

	switch(m_valuetype)
	{
	case AX_INT8:
		_snwprintf(temp, TEMP_STR_SIZE, L"%d", *((__int8*)m_valueBuf)); break;
	case AX_UINT8:
		_snwprintf(temp, TEMP_STR_SIZE, L"%u", *((unsigned __int8*)m_valueBuf)); break;
	case AX_INT16:
		_snwprintf(temp, TEMP_STR_SIZE, L"%d", *((__int16*)m_valueBuf)); break;
	case AX_UINT16:
		_snwprintf(temp, TEMP_STR_SIZE, L"%u", *((unsigned __int16*)m_valueBuf));	break;
	case AX_INT32:
		_snwprintf(temp, TEMP_STR_SIZE, L"%d", *((__int32*)m_valueBuf));	break;
	case AX_UINT32:
		_snwprintf(temp, TEMP_STR_SIZE, L"%u", *((unsigned __int32*)m_valueBuf));	break;
	case AX_FLOAT32:
		_snwprintf(temp, TEMP_STR_SIZE, L"%.8f", *((float*)m_valueBuf)); break;
	case AX_INT64:
		_snwprintf(temp, TEMP_STR_SIZE, L"%I64d", *((__int64*)m_valueBuf));	break;
	case AX_UINT64:
		_snwprintf(temp, TEMP_STR_SIZE, L"%I64u", *((unsigned __int64*)m_valueBuf));	break;
	case AX_FLOAT64:
		_snwprintf(temp, TEMP_STR_SIZE, L"%.16f", *((double*)m_valueBuf)); break;
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
