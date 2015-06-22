/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once
#include "Resource.h"
#include "AT_ListCtrlEx.h"
#include "AT_ChildFrame.h"
#include "AT_Util.h"

namespace AT3
{
class CTickDialog;
/** ChildFrame - Trace text
*/
class TickFrameWnd : public CMDIChildWindowImpl<TickFrameWnd>, public IChildFrame
{
public:
	/** get child type */
	virtual CHILD_STYLE getChildType(void) { return CS_TICK_FRAME; }
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
	DECLARE_FRAME_WND_CLASS(NULL, IDR_TICK_FRAME_TYPE)

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	BEGIN_MSG_MAP(TraceFrameWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)

		CHAIN_MSG_MAP(CMDIChildWindowImpl<TickFrameWnd>)
		CHAIN_CLIENT_COMMANDS()
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/*************************************************************************
		Implementation Data
	*************************************************************************/
private:
	CUpdateUIBase* m_pUpdateUI;
	std::string m_windowTitle;
	CTickDialog* m_wndTickWnd;

	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
public:
	TickFrameWnd(CUpdateUIBase* pUpdateUI, const std::string& windowTitle);
	~TickFrameWnd();
};

}
