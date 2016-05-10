#include "StdAfx.h"
#include "AT_System.h"
#include "AT_Config.h"
#include "AT_MainFrame.h"
#include "AT_Incoming.h"
#include "AT_MessageQueue.h"
#include "AT_TraceFrame.h"
#include "AT_ValueFrame.h"
#include "AT_Filter.h"

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
	, m_pIncoming(0)
	, m_msgQueue(0)
	, m_filter(0)
{
	s_pMe = this;
}

//--------------------------------------------------------------------------------------------
System::~System()
{
}

//--------------------------------------------------------------------------------------------
bool System::init(HINSTANCE hInstance, LPSTR lpCmdLine)
{
	m_hInstance = hInstance;
	m_strCmdLine = lpCmdLine;
	
	m_theConfig = new Config;
	m_wndMainFrame = new MainFrame;
	m_pIncoming = new Incoming;
	m_msgQueue = new MessageQueue;
	m_filter = new Filter;

	// Init WTL app module
	::InitCommonControls();
	m_AppModule.Init(NULL, m_hInstance);

	// try load system setting from regist
	m_theConfig->loadSetting();

	// init filter
	m_filter->init(m_theConfig);
	if (!(m_filter->reloadScript(m_theConfig->getFilterScript().c_str(), ::GetDesktopWindow()))) {
		return false;
	}

	//init receive thread
	return m_pIncoming->init();
}

//--------------------------------------------------------------------------------------------
void System::run(void)
{
	m_AppModule.AddMessageLoop(&m_theMsgLoop);

	// Create Main Frame and show it
	if(m_wndMainFrame->CreateEx() == NULL) return;

	m_theMsgLoop.AddMessageFilter(m_wndMainFrame);
	m_theMsgLoop.AddIdleHandler(m_wndMainFrame);
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

			delete msg; 
			msgVector[i] = 0;
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
	case AXTRACE_CMD_TYPE_TRACE:	//string log
		_insertStringLog((LogMessage*)message);
		break;

	case AXTRACE_CMD_TYPE_VALUE:	//value
		_watchValue((ValueMessage*)message);
		break;

	default: break;
	}
}

//--------------------------------------------------------------------------------------------
void System::_insertStringLog(const LogMessage* message)
{
	Filter::Result filterResult;
	m_filter->onTraceMessage(message, filterResult);

	if (!filterResult.display) return;

	TraceFrameWnd* logWnd = m_wndMainFrame->getTraceWnd(filterResult.wndTitle);
	assert(logWnd!=0);

	logWnd->insertLog(message, filterResult);
}

//--------------------------------------------------------------------------------------------
void System::_watchValue(const ValueMessage* message)
{
	Filter::Result filterResult;
	m_filter->onValueMessage(message, filterResult);

	if (!filterResult.display) return;

	ValueFrameWnd* valueWnd = m_wndMainFrame->getValueWnd(filterResult.wndTitle);
	assert(valueWnd!=0);

	std::wstring value;
	message->getValueAsString(value);
	valueWnd->watchValue(message->getStyleID(), message->getTraceTime(), message->getValueName(), value.c_str(), filterResult);
}

}