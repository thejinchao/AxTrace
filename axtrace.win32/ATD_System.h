/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#pragma once

/** static global value */
struct GlobalValue
{
	enum { MAX_SERVER_ADDR_LENGTH = 128 };

	bool	isInited;		//!< is inited
	bool	isInitSucc;		//!< if already init, is successed?
	//!< critical section for init
	CRITICAL_SECTION criticalSection;

	void*	zmqContex;		//!< contex of zeromq
	char	szServer[MAX_SERVER_ADDR_LENGTH];	//!< axtrace server addr
	int		nListenPort;	//!< axtrace listen port
	DWORD	dwTlsIndex;		//!< tls index
};

/** tls value */
struct ThreadValue
{
	bool	isInited;		//!< is inited
	bool	isInitSucc;		//!< if already init, is successed?

	void*	zmqSocket;		//!< socket of zeromq, because zeromq sockets are not threadsafe, it must be stored in tls block
};

/** get global value*/
GlobalValue& getGlobalValue(void);

/** init when very first time(dll be loaded)*/
bool _OnProcessAttached(void);

/** when process detached*/
void _OnProcessDetached(void);

/** try init global value*/
bool _InitGlobalValue(GlobalValue& global, const char* szTraceServer, int nTracePort);

/** try init thread value value*/
bool _InitThreadValue(GlobalValue& global);

/** get thread value from tls block */
ThreadValue* _GetThreadValue(GlobalValue& global);

/** relloc memory of tls */
bool _Relloc_TLSMemory(ThreadValue& thread, size_t size);

/* get listen port from common cookie memory file(-1 means failed) */
int _GetListenPort(void);
