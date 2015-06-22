/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_System.h"
#include "AT_MainFrame.h"
#include "AT_AboutDlg.h"
#include "AT_TraceFrame.h"
#include "AT_ValueFrame.h"
#include "AT_OptionsDlg.h"
#include "AT_TickFrame.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
MainFrame::MainFrame()
	: m_mdiStatus(MS_MDI)
	, m_currentActiveChild(0)
{
}

//--------------------------------------------------------------------------------------------
MainFrame::~MainFrame()
{
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// Create MDIClient
	CreateMDIClient();

	// Set Main Title
	SetWindowText(_T("AxTrace"));

	// Create MDI CommandBar
	m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	m_CmdBar.SetMDIClient(m_hWndClient);
	m_CmdBar.AttachMenu(GetMenu());
	SetMenu(NULL);

	m_hMainMenu = LoadMenu(System::getSingleton()->getAppModule().m_hInst, MAKEINTRESOURCE(IDR_MAINFRAME));
	m_hChildMenu = LoadMenu(System::getSingleton()->getAppModule().m_hInst, MAKEINTRESOURCE(IDR_TRACEFRAME));

	// Create Toolbar
	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	UIAddToolBar(hWndToolBar);

	// Create Rebar
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(m_CmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	// Set Menu status
	updateButtons(MS_MDI);
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DestroyMenu(m_hChildMenu);
	DestroyMenu(m_hMainMenu);
	bHandled=FALSE;
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnAxTraceMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	System::getSingleton()->OnIdle();
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnAppExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	PostMessage(WM_CLOSE);
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnSystemRecive(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Config* config = System::getSingleton()->getConfig();
	config->setCapture(!(config->getCapture()));
	updateButtons();
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnSystemAutoscroll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Config* config = System::getSingleton()->getConfig();
	config->setAutoScroll(!(config->getAutoScroll()));
	updateButtons();
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnActiveWndEditCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(m_currentActiveChild)
	{
		SendMessage(m_currentActiveChild->getNativeWnd(), WM_COMMAND, MAKELONG(wID, wNotifyCode), (LPARAM)hWndCtl);
	}
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnAllWndEditCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TraceWndMap::iterator it_trace, end_trace=m_traceWndMap.end();
	for(it_trace=m_traceWndMap.begin(); it_trace!=end_trace; it_trace++)
	{
		SendMessage(((IChildFrame*)it_trace->second)->getNativeWnd(), WM_COMMAND, MAKELONG(wID, wNotifyCode), (LPARAM)hWndCtl);
	}

	ValueWndMap::iterator it_value, end_value=m_valueWndMap.end();
	for(it_value=m_valueWndMap.begin(); it_value!=end_value; it_value++)
	{
		SendMessage(((IChildFrame*)it_value->second)->getNativeWnd(), WM_COMMAND, MAKELONG(wID, wNotifyCode), (LPARAM)hWndCtl);
	}
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnAppAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	AboutDlg dlg;
	dlg.DoModal();
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnOptionSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OptionsDlg dlg(*(System::getSingleton()->getConfig()));
	if(IDOK!=dlg.DoModal()) return 0;

	//update setting
	System::getSingleton()->getConfig()->copyFrom(dlg.getSetting());
	System::getSingleton()->getConfig()->saveSetting();

	//redraw all frame
	TraceWndMap::iterator it_trace, end_trace=m_traceWndMap.end();
	for(it_trace=m_traceWndMap.begin(); it_trace!=end_trace; it_trace++)
	{
		((IChildFrame*)it_trace->second)->redraw();
	}

	ValueWndMap::iterator it_value, end_value=m_valueWndMap.end();
	for(it_value=m_valueWndMap.begin(); it_value!=end_value; it_value++)
	{
		((IChildFrame*)it_value->second)->redraw();
	}

	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnMDISetMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	updateButtons(MS_MDI);
//	SetMenu(m_hMainMenu);
	m_CmdBar.AttachMenu(m_hMainMenu);

	bHandled = FALSE;
	return 1;
}

//--------------------------------------------------------------------------------------------
void MainFrame::onChildActive(IChildFrame* child)
{
	m_currentActiveChild = child;

	if(m_currentActiveChild->getChildType()==IChildFrame::CS_TRACE_FRAME)
	{
		m_CmdBar.AttachMenu(m_hChildMenu);
		updateButtons(MS_TRACE_FRAME);
	}
	else if(m_currentActiveChild->getChildType()==IChildFrame::CS_VALUE_FRAME)
	{
		m_CmdBar.AttachMenu(m_hChildMenu);
		updateButtons(MS_VALUE_FRAME);
	}
	else if(m_currentActiveChild->getChildType()==IChildFrame::CS_TICK_FRAME)
	{
		m_CmdBar.AttachMenu(m_hChildMenu);
		updateButtons(MS_TICK_FRAME);
	}
}

//--------------------------------------------------------------------------------------------
void MainFrame::onChildDestroy(IChildFrame* child)
{
	assert(child);
	if(child==0) return;
	
	if(m_currentActiveChild==child) m_currentActiveChild=0;
	if(child->getChildType()==IChildFrame::CS_TRACE_FRAME)
	{
		m_traceWndMap.erase(child->getWindowTitle());
	}
	else if(child->getChildType()==IChildFrame::CS_VALUE_FRAME)
	{
		m_valueWndMap.erase(child->getWindowTitle());
	}
	else if(child->getChildType()==IChildFrame::CS_TICK_FRAME)
	{
		m_tickWndMap.erase(child->getWindowTitle());
	}
}

//--------------------------------------------------------------------------------------------
void MainFrame::updateButtons(MDI_STATUS status)
{
	if(status!=MS_UNKNOWN) m_mdiStatus=status;
	else status = m_mdiStatus;

	//-----
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UISetCheck(ID_SYSTEM_RECEIVE, System::getSingleton()->getConfig()->getCapture());
	UISetCheck(ID_SYSTEM_AUTOSCROLL, System::getSingleton()->getConfig()->getAutoScroll());
	UIEnable(ID_HELP, FALSE); //NOT SUPPORT YET...
	UIEnable(ID_OPTION_SETTING, FALSE);

	if(status==MS_MDI)
	{
		UIEnable(ID_FILE_SAVEAS, FALSE);
		UIEnable(ID_SYSTEM_AUTOSCROLL, FALSE);
		UIEnable(ID_EDIT_CLEAR, FALSE);
		UIEnable(ID_EDIT_CLEARALL, FALSE);
	}
	else if(status==MS_TRACE_FRAME)
	{
		UIEnable(ID_FILE_SAVEAS, TRUE);
		UIEnable(ID_SYSTEM_AUTOSCROLL, TRUE);
		UIEnable(ID_EDIT_CLEAR, TRUE);
		UIEnable(ID_EDIT_CLEARALL, TRUE);
	}
	else if(status==MS_VALUE_FRAME)
	{
		UIEnable(ID_FILE_SAVEAS, TRUE);
		UIEnable(ID_SYSTEM_AUTOSCROLL, FALSE);
		UIEnable(ID_EDIT_CLEAR, TRUE);
		UIEnable(ID_EDIT_CLEARALL, TRUE);
	}
}

//--------------------------------------------------------------------------------------------
TraceFrameWnd* MainFrame::getTraceWnd(const std::string& windowTitle)
{
	TraceWndMap::iterator it = m_traceWndMap.find(windowTitle);
	if(it!=m_traceWndMap.end()) return it->second;

	wchar_t temp[64]={0};
	StringCchPrintfW(temp, 64, _T("Log:%s"), windowTitle.c_str());

	TraceFrameWnd* pChild = new TraceFrameWnd((CUpdateUIBase*)this, windowTitle);
	pChild->CreateEx(m_hWndClient, NULL, temp);

	m_traceWndMap.insert(std::make_pair(windowTitle, pChild));
	return pChild;
}

//--------------------------------------------------------------------------------------------
ValueFrameWnd* MainFrame::getValueWnd(const std::string& windowTitle)
{
	ValueWndMap::iterator it = m_valueWndMap.find(windowTitle);
	if(it!=m_valueWndMap.end()) return it->second;

	wchar_t temp[64]={0};
	StringCchPrintfW(temp, 64, _T("Value:%s"), windowTitle.c_str());

	ValueFrameWnd* pChild = new ValueFrameWnd((CUpdateUIBase*)this, windowTitle);
	pChild->CreateEx(m_hWndClient, NULL, temp);

	m_valueWndMap.insert(std::make_pair(windowTitle, pChild));
	return pChild;
}

//--------------------------------------------------------------------------------------------
TickFrameWnd* MainFrame::getTickWnd(const std::string& windowTitle)
{
	TickWndMap::iterator it = m_tickWndMap.find(windowTitle);
	if(it!=m_tickWndMap.end()) return it->second;

	wchar_t temp[64]={0};
	StringCchPrintfW(temp, 64, _T("Tick:%s"), windowTitle.c_str());

	TickFrameWnd* pChild = new TickFrameWnd((CUpdateUIBase*)this, windowTitle);
	pChild->CreateEx(m_hWndClient, NULL, temp);

	m_tickWndMap.insert(std::make_pair(windowTitle, pChild));
	return pChild;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnOptionHideToolBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	BOOL bNew = !::IsWindowVisible(m_hWndToolBar);
	::ShowWindow(m_hWndToolBar, bNew ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_OPTION_HIDE_TOOLBAR, !bNew);
	SetMenu(bNew ? 0 : (m_currentActiveChild ? m_hChildMenu : m_hMainMenu));
	
	UpdateLayout();
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnWindowCascade(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	MDICascade(0);
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnWindowTile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	MDITile(MDITILE_HORIZONTAL);
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT MainFrame::OnSettingFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	LOGFONT lf;
	GetObject(System::getSingleton()->getConfig()->getFont(), sizeof(lf), &lf);

	CFontDialog dlg(&lf, CF_SCREENFONTS, 0, m_hWnd);
	if(IDOK!=dlg.DoModal()) return 0;

	dlg.GetCurrentFont(&lf);

	System::getSingleton()->getConfig()->setFont(&lf);
	System::getSingleton()->getConfig()->saveSetting();

	//redraw all frame
	TraceWndMap::iterator it_trace, end_trace=m_traceWndMap.end();
	for(it_trace=m_traceWndMap.begin(); it_trace!=end_trace; it_trace++)
	{
		((IChildFrame*)it_trace->second)->redraw();
	}

	ValueWndMap::iterator it_value, end_value=m_valueWndMap.end();
	for(it_value=m_valueWndMap.begin(); it_value!=end_value; it_value++)
	{
		((IChildFrame*)it_value->second)->redraw();
	}
	return 0;
}

}