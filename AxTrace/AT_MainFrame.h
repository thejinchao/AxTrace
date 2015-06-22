/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once
#include "resource.h"

namespace AT3
{
//pre-define
class TraceFrameWnd;
class ValueFrameWnd;
class TickFrameWnd;
class IChildFrame;

/** Main Frame
*/
class MainFrame : public CMDIFrameWindowImpl<MainFrame>, CUpdateUI<MainFrame>,
		public CMessageFilter, public CUpdateUIObject
{
	/*************************************************************************
			Inherit Methods
	*************************************************************************/
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
	enum { WM_ON_AXTRACE_MESSAGE=WM_USER+100 };

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CMDIFrameWindowImpl<MainFrame>::PreTranslateMessage(pMsg);
	}

	virtual BOOL DoUpdate()
	{
		UIUpdateToolBar();
		return FALSE;
	}

	BEGIN_MSG_MAP(MainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_ON_AXTRACE_MESSAGE, OnAxTraceMessage)
		
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnAppExit)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_SYSTEM_RECEIVE, OnSystemRecive)
		COMMAND_ID_HANDLER(ID_SYSTEM_AUTOSCROLL, OnSystemAutoscroll)
		COMMAND_ID_HANDLER(ID_OPTION_SETTING, OnOptionSetting)
		COMMAND_ID_HANDLER(ID_WINDOW_CASCADE, OnWindowCascade)
		COMMAND_ID_HANDLER(ID_WINDOW_TILE_HORZ, OnWindowTile)
		COMMAND_ID_HANDLER(ID_OPTION_FONT, OnSettingFont)
		COMMAND_ID_HANDLER(ID_OPTION_HIDE_TOOLBAR, OnOptionHideToolBar)
		

		COMMAND_ID_HANDLER(ID_EDIT_CLEAR, OnActiveWndEditCommand)
		COMMAND_ID_HANDLER(ID_EDIT_CLEARALL, OnAllWndEditCommand)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnActiveWndEditCommand)
		COMMAND_ID_HANDLER(ID_EDIT_SELECTALL, OnActiveWndEditCommand)
		COMMAND_ID_HANDLER(ID_FILE_SAVEAS, OnActiveWndEditCommand)

		MESSAGE_HANDLER(WM_MDISETMENU, OnMDISetMenu)
		CHAIN_MDI_CHILD_COMMANDS()
		CHAIN_MSG_MAP(CUpdateUI<MainFrame>)
		CHAIN_MSG_MAP(CMDIFrameWindowImpl<MainFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(MainFrame)
		UPDATE_ELEMENT(ID_OPTION_HIDE_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_HELP, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_OPTION_SETTING, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_FILE_SAVEAS, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_SYSTEM_RECEIVE,  UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_SYSTEM_AUTOSCROLL,  UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_CLEAR,  UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_CLEARALL,  UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	/** On MainFrame Create
	*/
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/** On MainFrame Create
	*/
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/** On Command ID_APP_ABOUT
	*/
	LRESULT OnMDISetMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAxTraceMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOptionHideToolBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAppExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSystemRecive(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSystemAutoscroll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAppAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnActiveWndEditCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAllWndEditCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOptionSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnWindowCascade(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnWindowTile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSettingFont(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
	enum MDI_STATUS
	{
		MS_UNKNOWN,

		MS_MDI,
		MS_TRACE_FRAME,
		MS_VALUE_FRAME,
		MS_TICK_FRAME,
	};
	/** Update ui buttons on menu and toolbar */
	void updateButtons(MDI_STATUS status=MS_UNKNOWN);

public:
	TraceFrameWnd* getTraceWnd(const std::string& windowTitle);
	ValueFrameWnd* getValueWnd(const std::string& windowTitle);
	TickFrameWnd* getTickWnd(const std::string& windowTitle);

	void onChildActive(IChildFrame* child);
	void onChildDestroy(IChildFrame* child);

	/*************************************************************************
		Implementation Data
	*************************************************************************/
private:
	CMDICommandBarCtrl		m_CmdBar;		//!< Command Bar Control.

	typedef std::map< std::string, TraceFrameWnd* > TraceWndMap;
	TraceWndMap				m_traceWndMap;

	typedef std::map< std::string, ValueFrameWnd* > ValueWndMap;
	ValueWndMap				m_valueWndMap;

	typedef std::map< std::string, TickFrameWnd* > TickWndMap;
	TickWndMap				m_tickWndMap;

	MDI_STATUS				m_mdiStatus;
	IChildFrame*			m_currentActiveChild;

	HMENU					m_hMainMenu;
	HMENU					m_hChildMenu;

	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
public:
	MainFrame();
	~MainFrame();
};

}
