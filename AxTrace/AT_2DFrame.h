/***************************************************

			AXIA|Trace3

		(C) Copyright  Jean. 2016
***************************************************/
#pragma once
#include "Resource.h"
#include "AT_ChildFrame.h"
#include "AT_Util.h"
#include "AT_Filter.h"
#include "AT_2DView.h"
#include "AT_Math.h"

namespace AT3
{
//pre-define
class LogMessage;
class Graphics2DScene;
class Graphics2DCamera;

/** ChildFrame - Trace text
*/
class Graphics2DFrameWnd : public CMDIChildWindowImpl<Graphics2DFrameWnd>, public IChildFrame
{
public:
	/** get child type */
	virtual CHILD_STYLE getChildType(void) { return CS_2D_FRAME; }
	/** get window title*/
	virtual const std::string& getWindowTitle(void) { return m_windowTitle; }
	/** redraw */
	virtual void redraw(void);

	virtual HWND getNativeWnd(void) { return m_hWnd; }
	Graphics2DCamera* getCamera(void) { return m_camera; }
	void cleanMap(double x_size, double y_size);

	void initGL(int screen_x, int screen_y);
	void render(void);

	/*************************************************************************
	Inherit Methods
	*************************************************************************/
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_TRACE_FRAME_TYPE)

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	BEGIN_MSG_MAP(Graphics2DFrameWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_ID_HANDLER(ID_EDIT_CLEAR, OnEditClear)
		COMMAND_ID_HANDLER(ID_EDIT_CLEARALL, OnEditClear)
		CHAIN_MSG_MAP(CMDIChildWindowImpl<Graphics2DFrameWnd>)
		CHAIN_CLIENT_COMMANDS()
		CHAIN_MSG_MAP_MEMBER(m_wndCanvas)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEditClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	static void _onCameraAdjust(int sreenWidth, int screenHeight, const fVector2& lowPos, const fVector2& upperPos, void* param);

	/*************************************************************************
	Implementation Data
	*************************************************************************/
private:
	Graphics2DView m_wndCanvas;
	CUpdateUIBase* m_pUpdateUI;
	std::string m_windowTitle;
	Graphics2DScene* m_scene;
	Graphics2DCamera* m_camera;
	fVector2 m_lowPos;
	fVector2 m_upperPos;

	/*************************************************************************
	Construction and Destruction
	*************************************************************************/
public:
	Graphics2DFrameWnd(CUpdateUIBase* pUpdateUI, const std::string& windowTitle);
	~Graphics2DFrameWnd();
};

}