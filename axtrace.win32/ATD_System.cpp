/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "stdafx.h"
#include "ATD_System.h"
#include "ATD_Interface.h"

//--------------------------------------------------------------------------------------------
GlobalValue& getGlobalValue(void)
{
	static GlobalValue s_theGlobalValue={0};		//all static value of this dll is here!
	return s_theGlobalValue;
}

//--------------------------------------------------------------------------------------------
bool _OnProcessAttached(void)
{
	GlobalValue& global = getGlobalValue();

	global.isInited = 0;
	global.isInitSucc = 0;
	InitializeCriticalSection(&global.criticalSection);
	global.zmqContex = 0;
	global.dwTlsIndex = TlsAlloc();
	if(global.dwTlsIndex == TLS_OUT_OF_INDEXES) return false;

	return true;
}

//--------------------------------------------------------------------------------------------
void _OnProcessDetached(void)
{
}

//--------------------------------------------------------------------------------------------
bool _InitGlobalValue(GlobalValue& global, const char* szTraceServer, int nTracePort)
{
	//already inited?
	if(global.isInited) return global.isInitSucc;

	//try init,first enter critical section
	EnterCriticalSection(&global.criticalSection);
	__try
	{
		//already inited by other thread?
		if(global.isInited) return global.isInitSucc;

		global.isInitSucc = false; //as default result

		//---------------
		//0. get server addr and listen port
		if (szTraceServer == 0 || nTracePort == 0)
		{
			//from share memory
			StringCbCopyA(global.szServer, global.MAX_SERVER_ADDR_LENGTH, "127.0.0.1");
			global.nListenPort = _GetListenPort();
			if (global.nListenPort <= 0) return false;	// get listen port error
		}
		else
		{
			StringCbCopyA(global.szServer, global.MAX_SERVER_ADDR_LENGTH, szTraceServer);
			global.nListenPort = nTracePort;
		}
		//---------------
		//1: init zeromq
		global.zmqContex = zmq_ctx_new();
		if(global.zmqContex==0) return false;	// init zeromq failed

		//ok we are success!
		global.isInitSucc = true;
	}
	__finally
	{
		//already try inited, and won't do this again!
		global.isInited = true; 
		//failed?
		if(!global.isInitSucc)
		{
			if(global.zmqContex!=0) { zmq_ctx_destroy(global.zmqContex); global.zmqContex=0; }
		}
		//leave critical section
		LeaveCriticalSection(&global.criticalSection);
	}

	//success!
	return global.isInitSucc;
}

//--------------------------------------------------------------------------------------------
ThreadValue* _GetThreadValue(GlobalValue& global)
{
	ThreadValue* tls = (ThreadValue*)TlsGetValue(global.dwTlsIndex);
	if(tls!=0) return tls;

	//alloc thread value block
	tls = (ThreadValue*)LocalAlloc(LPTR, sizeof(ThreadValue));
	tls->zmqSocket = 0;
	tls->isInited = false;
	tls->isInitSucc = false;

	TlsSetValue(global.dwTlsIndex, tls);
	return tls;
}

//--------------------------------------------------------------------------------------------
bool _InitThreadValue(GlobalValue& global)
{
	ThreadValue* tls = _GetThreadValue(global);
	if(tls->isInited) return tls->isInitSucc;

	//try init
	__try
	{
		tls->isInitSucc = false;

		//create socket
		tls->zmqSocket = zmq_socket(global.zmqContex, ZMQ_PUSH);
		if(tls->zmqSocket==0) return 1;	// init zeromq failed

		//connect to listen port 
		char temp[64]={0};
		StringCchPrintfA(temp, 64, "tcp://%s:%d", global.szServer, global.nListenPort);
		if(0!=zmq_connect(tls->zmqSocket, temp)) return 1;

		tls->isInitSucc = true;
	}
	__finally
	{
		tls->isInited = true;
	}

	return tls->isInitSucc;
}

//--------------------------------------------------------------------------------------------
bool _Relloc_TLSMemory(ThreadValue& thread, size_t size)
{
	return true;
}

//--------------------------------------------------------------------------------------------
int _GetListenPort(void)
{
	HANDLE hMapFile = OpenFileMappingW(FILE_MAP_READ, FALSE, AXTRACE_COMMON_COOKIE_FILENAME);
	if(hMapFile==0) return -1;

	AXTRACE_COMMON_COOKIE* cookie = (AXTRACE_COMMON_COOKIE*)MapViewOfFileEx(hMapFile, FILE_MAP_READ, 0, 0, 0, 0);
	if(cookie==0) 
	{
		CloseHandle(hMapFile);
		return -1;
	}

	int port = cookie->nListenPort;

	UnmapViewOfFile(cookie); cookie=0;
	CloseHandle(hMapFile);

	return port;
}
