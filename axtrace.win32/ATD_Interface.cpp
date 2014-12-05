/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/


#include "stdafx.h"
#include "ATD_Interface.h"
#include "ATD_System.h"

//--------------------------------------------------------------------------------------------
int __stdcall AxTrace_Init(const char* szTraceServer, int nTracePort)
{
	GlobalValue& global = getGlobalValue();

	//init global value
	if(!_InitGlobalValue(global, szTraceServer, nTracePort)) return 1;
	//init thread value
	if(!_InitThreadValue(global)) return 1;

	return 0;
}

//--------------------------------------------------------------------------------------------
int _InsertLog(int idWindow, int idStyle, int codePage, const void* stringBuf, int stringLengthInBytes)
{
	//init faile or already failed?
	if(AxTrace_Init()!=0) return 1;

	//check range
	if(idWindow<0 || idWindow>AT_MaxWindowID) return 1;
	if(idStyle<0 || idStyle>AT_MaxStyleID) return 1;
	if(codePage!=ATC_ACP && codePage!=ATC_UTF8 && codePage!=ATC_UTF16) return 1;
	if(stringBuf==0) return false;
	if(stringLengthInBytes<=0 || stringLengthInBytes> AT_MaxLogLengthInByte) return 1;

	//get thread value
	GlobalValue& global = getGlobalValue();
	ThreadValue* tls = _GetThreadValue(global);

	//final length
	size_t final_length = sizeof(int) + sizeof(int) + stringLengthInBytes;

	//build message 
	AXIATRACE_DATAHEAD head;
	head.MAGIC_HEAD = AXTRACE_MAGIC_HEAD;
	head.dwProcessID = ::GetCurrentProcessId();
	head.dwThreadID = ::GetCurrentThreadId();
	head.wTraceType = ATT_LOG;
	head.cWinID = (unsigned char)(idWindow);
	head.cStyleID = (unsigned char)(idStyle);
	head.wContentLen = (unsigned short)final_length;

	zmq_msg_t msg;
    int rc = zmq_msg_init_size(&msg, sizeof(head)+final_length);
	if(rc!=0) return 1;

	char* msg_data = (char*)zmq_msg_data(&msg);
	//0. write head
    memcpy(msg_data, &head, sizeof(head));	msg_data += sizeof(head);
	//1. write string codepage
	memcpy(msg_data, &codePage, sizeof(int)); msg_data += sizeof(int);
	//2. write string length
	memcpy(msg_data, &stringLengthInBytes, sizeof(int)); msg_data += sizeof(int);
	//3. write string 
	memcpy(msg_data, stringBuf, stringLengthInBytes); 

	bool success = (sizeof(head)+final_length == zmq_msg_send(&msg, tls->zmqSocket, ZMQ_DONTWAIT));

	zmq_msg_close(&msg);

	return success ? 0 : 1;
}

//--------------------------------------------------------------------------------------------
int __stdcall AxTrace_InsertLogA(int idWindow, int idStyle, int isUTF8, const char* stringBuf)
{
	size_t lengthInBytes = 0;
	StringCbLengthA(stringBuf, AT_MaxLogLengthInByte, &lengthInBytes);
	return _InsertLog(idWindow, idStyle, (isUTF8 != 0 ? ATC_UTF8 : ATC_ACP), stringBuf, (int)lengthInBytes);
}

//--------------------------------------------------------------------------------------------
int __stdcall AxTrace_InsertLogW(int idWindow, int idStyle, const unsigned short* stringBuf)
{
	size_t lengthInBytes = 0;
	StringCbLengthW((const wchar_t*)stringBuf, AT_MaxLogLengthInByte, &lengthInBytes);
	return _InsertLog(idWindow, idStyle, ATC_UTF16, stringBuf, (int)lengthInBytes);
}

//--------------------------------------------------------------------------------------------
size_t _getValueLength(AXTRACE_VALUE_TYPE valueType, const void* value)
{
	switch(valueType)
	{
	case AX_INT8: case AX_UINT8: return 1;
	case AX_INT16: case AX_UINT16: return 2;
	case AX_INT32: case AX_UINT32: return 4;
	case AX_INT64: case AX_UINT64: return 8;
	case AX_FLOAT32: return 4;
	case AX_FLOAT64: return 8;
	case AX_STR_ACP: case AX_STR_UTF8: return strlen((const char*)value)+1;
	case AX_STR_UTF16: return 2*(wcslen((const wchar_t*)value)+1);
	default: break;
	}
	return 0;
}

//--------------------------------------------------------------------------------------------
int __stdcall AxTrace_WatchValue(int idWindow, int idStyle, int valueType, const char* name, int nameLengthInBytes, const void* value)
{
	//init faile or already failed?
	if(AxTrace_Init()!=0) return 1;

	//check range
	if(idWindow<0 || idWindow>AT_MaxWindowID) return 1;
	if(idStyle<0 || idStyle>AT_MaxStyleID) return 1;
	if(valueType<0 || valueType>AX_MAX_VALUETYPE) return 1;

	if(name == 0) return false;
	if(nameLengthInBytes <= 0 || nameLengthInBytes> AT_MaxValueNameLength) return 1;
	if(value==0) return false;

	//get thread value
	GlobalValue& global = getGlobalValue();
	ThreadValue* tls = _GetThreadValue(global);

	//calc final length
	size_t value_length = _getValueLength((AXTRACE_VALUE_TYPE)valueType, value);
	size_t final_length = sizeof(valueType) + sizeof(nameLengthInBytes) + nameLengthInBytes + sizeof(value_length) + value_length;

	//build message 
	AXIATRACE_DATAHEAD head;
	head.MAGIC_HEAD = AXTRACE_MAGIC_HEAD;
	head.dwProcessID = ::GetCurrentProcessId();
	head.dwThreadID = ::GetCurrentThreadId();
	head.wTraceType = ATT_VALUE;
	head.cWinID = (unsigned char)(idWindow);
	head.cStyleID = (unsigned char)(idStyle);
	head.wContentLen = (unsigned short)final_length;

	zmq_msg_t msg;
    int rc = zmq_msg_init_size(&msg, sizeof(head)+final_length);
	if(rc!=0) return 1;

	char* msg_data = (char*)zmq_msg_data(&msg);
	//0. write head
    memcpy((void*)msg_data, &head, sizeof(head)); msg_data += sizeof(head);
	//1. write value type
	memcpy((void*)msg_data, &valueType, sizeof(int)); msg_data += sizeof(int);
	//2. write value name length
	memcpy((void*)msg_data, &nameLengthInBytes, sizeof(int)); msg_data += sizeof(int);
	//3. write value name
	memcpy((void*)msg_data, name, nameLengthInBytes); msg_data += nameLengthInBytes;
	//4. write value length
	memcpy((void*)msg_data, &value_length, sizeof(value_length)); msg_data += sizeof(value_length);
	//5. write value
	memcpy((void*)msg_data, value, value_length);

	bool success = (sizeof(head)+final_length == zmq_msg_send(&msg, tls->zmqSocket, 0));

	zmq_msg_close(&msg);
	return success ? 0 : 1;
}
