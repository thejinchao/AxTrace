/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_TickFrame.h"
#include "AT_Util.h"
#include "AT_System.h"
#include "AT_MainFrame.h"
#include "AT_Config.h"

namespace AT3
{

//////////////////////////////////////////////////////////////////////////////////////////////
//CTickDialog
//////////////////////////////////////////////////////////////////////////////////////////////
class CTickDialog : public CDialogImpl<CTickDialog>, public CDialogResize<CTickDialog>
{
public:
	enum { IDD = IDD_WND_TICK_FRAME };

	BEGIN_MSG_MAP(CTickDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOKCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancelCmd)

		CHAIN_MSG_MAP(CDialogResize<CTickDialog>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CTickDialog)
		DLGRESIZE_CONTROL(IDC_TREE_NODE, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CANVAS, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	/** Init Dialog
	*/
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		DlgResize_Init(false);

		m_treeNode = GetDlgItem(IDC_TREE_NODE);

		HTREEITEM hParent = m_treeNode.InsertItem( _T( "Top Level 1.1" ), 0, 1, TVI_ROOT, TVI_LAST );
		m_treeNode.InsertItem( _T( "Second Level 2.1" ), 0, 1, hParent, TVI_LAST );
		m_treeNode.InsertItem( _T( "Second Level 2.2" ), 0, 1, hParent, TVI_LAST );
		m_treeNode.Expand( hParent, TVE_EXPAND );


		hParent = m_treeNode.InsertItem( _T( "Top Level 1.2" ), 0, 1, TVI_ROOT, TVI_LAST );
		m_treeNode.InsertItem( _T( "Second Level 2.1" ), 0, 1, hParent, TVI_LAST );
		m_treeNode.InsertItem( _T( "Second Level 2.2" ), 0, 1, hParent, TVI_LAST );
		m_treeNode.Expand( hParent, TVE_EXPAND );

		return FALSE;
	}

	LRESULT OnOKCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		return TRUE;
	}

	LRESULT OnCancelCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		return TRUE;
	}

private:
	CTreeViewCtrl m_treeNode;

public:
	CTickDialog()
	{
	}
	virtual ~CTickDialog()
	{
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////
TickFrameWnd::TickFrameWnd(CUpdateUIBase* pUpdateUI, const std::string& windowTitle)
	: m_pUpdateUI(pUpdateUI)
	, m_windowTitle(windowTitle)
	, m_wndTickWnd(0)
{
	m_wndTickWnd = new CTickDialog;
}

//--------------------------------------------------------------------------------------------
TickFrameWnd::~TickFrameWnd()
{
	delete m_wndTickWnd; m_wndTickWnd=0;
}

//--------------------------------------------------------------------------------------------
LRESULT TickFrameWnd::OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	m_hWndClient = m_wndTickWnd->Create(m_hWnd);

	bHandled = TRUE;
	return 1;
}

//--------------------------------------------------------------------------------------------
LRESULT TickFrameWnd::OnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);
	if(width>0 && height>0 && m_wndTickWnd->m_hWnd!=0 )
	{
		RECT rect;
		GetClientRect(&rect);

		m_wndTickWnd->MoveWindow(0, 0, width, height);
	}
	bHandled = FALSE;
	return FALSE;
}

//--------------------------------------------------------------------------------------------
LRESULT TickFrameWnd::OnSetFocus(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	System::getSingleton()->getMainFrame()->onChildActive(this);
	return 1;
}

//--------------------------------------------------------------------------------------------
LRESULT TickFrameWnd::OnClose(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	System::getSingleton()->getMainFrame()->onChildDestroy(this);
	bHandled=FALSE;
	return 1;
}


//--------------------------------------------------------------------------------------------
void TickFrameWnd::redraw(void)
{
	m_wndTickWnd->InvalidateRect(0);
}


}
