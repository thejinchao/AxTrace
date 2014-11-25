#include "StdAfx.h"
#include "AT_System.h"
#include "AT_Config.h"
#include "AT_MainFrame.h"
#include "AT_Incoming.h"
#include "AT_MessageQueue.h"
#include "AT_TraceFrame.h"
#include "AT_ValueFrame.h"

#include <ATD_Interface.h>

namespace AT3
{

//////////////////////////////////////////////////////////////////////////////////////////////
//System
//////////////////////////////////////////////////////////////////////////////////////////////
System* System::s_pMe = 0;
//--------------------------------------------------------------------------------------------
System::System()
	: m_hMutex(0)
	, m_hInstance(0)
	, m_theConfig(0)
	, m_wndMainFrame(0)
	, m_zmpHandle(0)
	, m_pIncoming(0)
	, m_msgQueue(0)
	, m_hCookieMutex(0)
	, m_hCookieFile(0)
	, m_pCookie(0)
{
	s_pMe = this;
}

//--------------------------------------------------------------------------------------------
System::~System()
{
	if(m_pCookie)
	{
		UnmapViewOfFile(m_pCookie);
		m_pCookie=0;
	}

	if(m_hCookieFile)
	{
		CloseHandle(m_hCookieFile);
		m_hCookieFile=0;
	}

	if(m_hCookieMutex)
	{
		CloseHandle(m_hCookieMutex);
		m_hCookieMutex=0;
	}
}

//--------------------------------------------------------------------------------------------
bool System::init(HINSTANCE hInstance, LPSTR lpCmdLine)
{
	m_hInstance = hInstance;
	m_strCmdLine = lpCmdLine;
	
	if(!_createCommonCookie()) return false;

	m_theConfig = new Config;
	m_wndMainFrame = new MainFrame;
	m_pIncoming = new Incoming;
	m_msgQueue = new MessageQueue;

	// Init WTL app module
	::InitCommonControls();
	m_AppModule.Init(NULL, m_hInstance);

	// try load system setting from regist
	m_theConfig->loadSetting();

	// init zmp handle
	m_zmpHandle = zmq_ctx_new();

	//init receive thread
	m_pIncoming->init();

	return true;
}

//--------------------------------------------------------------------------------------------
void System::run(void)
{
	m_AppModule.AddMessageLoop(&m_theMsgLoop);

	// Create Main Frame and show it
	if(m_wndMainFrame->CreateEx() == NULL) return;

	m_theMsgLoop.AddMessageFilter(m_wndMainFrame);
	m_theMsgLoop.AddUpdateUI(m_wndMainFrame);
	m_theMsgLoop.AddIdleHandler(this);

	m_wndMainFrame->ShowWindow(SW_SHOWMAXIMIZED);

	// Run Message loop 
	m_theMsgLoop.Run();
}

//--------------------------------------------------------------------------------------------
void System::release(void)
{
	//close listen thread
	m_pIncoming->closeListen();
	// save setting to regist
	m_theConfig->saveSetting();

	// Close WTL App Module
	m_AppModule.RemoveMessageLoop();
	m_AppModule.Term();

	delete m_theConfig;		m_theConfig=0;
	delete m_wndMainFrame;	m_wndMainFrame=0;
	delete m_pIncoming;		m_pIncoming=0;
	delete m_msgQueue;		m_msgQueue=0;

	//destroy zeromp handle
	zmq_ctx_destroy(m_zmpHandle);
}

//--------------------------------------------------------------------------------------------
BOOL System::OnIdle(void)
{
	MessageVector msgVector;
	m_msgQueue->processMessage(msgVector);
	if(!msgVector.empty())
	{
		for(size_t i=0; i<msgVector.size(); i++)
		{
			const Message* msg = msgVector[i];

			_processAxTraceData(msg);

			delete[] msg;
		}
		msgVector.clear();
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
void System::_processAxTraceData(const Message* message)
{
	assert(message);

	switch(message->getTraceType())
	{
	case ATT_LOG:	//string log
		_insertStringLog((LogMessage*)message);
		break;

	case ATT_VALUE:	//value
		_watchValue((ValueMessage*)message);
		break;

	default: break;
	}
}

//--------------------------------------------------------------------------------------------
void System::_insertStringLog(const LogMessage* message)
{
	TraceFrameWnd* logWnd = m_wndMainFrame->getTraceWnd(message->getWindowID());
	assert(logWnd!=0);

	logWnd->insertLog(message->getStyleID(), message->getTraceTime(), message->getLogBuf(), message->getLogSizeChar());
}

//--------------------------------------------------------------------------------------------
void System::_watchValue(const ValueMessage* message)
{
	ValueFrameWnd* valueWnd = m_wndMainFrame->getValueWnd(message->getWindowID());
	assert(valueWnd!=0);

	std::wstring value;
	message->getValueAsString(value);
	valueWnd->watchValue(message->getStyleID(), message->getTraceTime(), message->getValueName(), value.c_str());
}

//--------------------------------------------------------------------------------------------
bool System::_createCommonCookie(void)
{
	//create global mutex
	m_hCookieMutex = CreateMutex(0, FALSE, _T("Global\\fa023401-93f6-468b-94e0-f40a35e7c760"));
	if(m_hCookieMutex!=0 && ::GetLastError()==ERROR_ALREADY_EXISTS)
	{
		//other process exist!
		MessageBox(0, _T("Another process already run!"), _T("AxTrace 3"), MB_OK|MB_ICONSTOP);
		return false;
	}

	if(m_hCookieMutex==0)
	{
		//TODO: information...
		return false;
	}

	//create memory file
	m_hCookieFile = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(AXTRACE_COMMON_COOKIE), AXTRACE_COMMON_COOKIE_FILENAME);
	if(m_hCookieFile==NULL)
	{
		//TODO: information
		return false;
	}

	//map view
	m_pCookie = (AXTRACE_COMMON_COOKIE*)MapViewOfFileEx(m_hCookieFile, FILE_MAP_ALL_ACCESS, 0, 0, 0, 0);
	if(!m_pCookie)
	{
		//TODO: information
		return false;
	}

	return true;
}
}