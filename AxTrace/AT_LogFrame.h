/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once
#include "Resource.h"
#include "AT_ListCtrlEx.h"
#include "AT_ChildFrame.h"
#include "AT_Util.h"
#include "AT_Filter.h"

namespace AT3
{
//pre-define
class LogMessage;

/** ChildFrame - Trace text
*/
class LogFrameWnd : public CMDIChildWindowImpl<LogFrameWnd>, public IChildFrame
{
public:
	/** insert a log message */
	void insertLog(const LogMessage* message, const Filter::Result& filter);
	/** get child type */
	virtual CHILD_STYLE getChildType(void) { return CS_TRACE_FRAME; }
	/** get window title*/
	virtual const std::string& getWindowTitle(void) { return m_windowTitle; }
	/** redraw */
	virtual void redraw(void);
	/** get native wnd handle */
	virtual HWND getNativeWnd(void) { return m_hWnd; }

	/*************************************************************************
			Inherit Methods
	*************************************************************************/
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_TRACE_FRAME_TYPE)

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	BEGIN_MSG_MAP(LogFrameWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_ID_HANDLER(ID_EDIT_CLEAR, OnEditClear)
		COMMAND_ID_HANDLER(ID_EDIT_CLEARALL, OnEditClear)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		COMMAND_ID_HANDLER(ID_EDIT_SELECTALL, OnEditSelectAll)
		COMMAND_ID_HANDLER(ID_FILE_SAVEAS, OnFileSaveAs)
		CHAIN_MSG_MAP(CMDIChildWindowImpl<LogFrameWnd>)
		CHAIN_CLIENT_COMMANDS()
		CHAIN_MSG_MAP_MEMBER(m_wndListView)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEditClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFileSaveAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	/*************************************************************************
		Implementation Data
	*************************************************************************/
private:
	CListCtrlEx m_wndListView;
	CUpdateUIBase* m_pUpdateUI;
	std::string m_windowTitle;
	int m_nLogIndex;
	AutoSizeBuf m_bufMaxLine;

	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
public:
	LogFrameWnd(CUpdateUIBase* pUpdateUI, const std::string& windowTitle);
	~LogFrameWnd();
};

}