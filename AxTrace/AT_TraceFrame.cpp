/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_TraceFrame.h"
#include "AT_Util.h"
#include "AT_System.h"
#include "AT_MainFrame.h"
#include "AT_Config.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
TraceFrameWnd::TraceFrameWnd(CUpdateUIBase* pUpdateUI, int windowID)
	: m_pUpdateUI(pUpdateUI)
	, m_windowID(windowID)
	, m_nLogIndex(0)
{

}

//--------------------------------------------------------------------------------------------
TraceFrameWnd::~TraceFrameWnd()
{

}

//--------------------------------------------------------------------------------------------
void TraceFrameWnd::redraw(void)
{
	m_wndListView.SetFont(System::getSingleton()->getConfig()->getFont());
	m_wndListView.InvalidateRect(0);
}

//--------------------------------------------------------------------------------------------
LRESULT TraceFrameWnd::OnEditCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_wndListView.copyToClipboard();
	return TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT TraceFrameWnd::OnEditClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_wndListView.DeleteAllItems();
	m_nLogIndex = 0;
	return TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT TraceFrameWnd::OnEditSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int count=m_wndListView.GetItemCount();
	for(int i=0; i<count; i++)
	{
		m_wndListView.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT TraceFrameWnd::OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	m_hWndClient = 
		m_wndListView.Create(m_hWnd, rcDefault, CListViewCtrl::GetWndClassName(), 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		LVS_REPORT | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER,
		WS_EX_CLIENTEDGE|LVS_EX_FULLROWSELECT);

	RECT rect;
	GetClientRect(&rect);

	// Create list view columns
	m_wndListView.InsertColumn(0, _T("#"), LVCFMT_LEFT, 40, 0);
	m_wndListView.InsertColumn(1, _T("Time"), LVCFMT_LEFT, 90, 0);
	int remainWidth = rect.right-rect.left-(40+90+32);
	m_wndListView.InsertColumn(2, _T("Log"), LVCFMT_LEFT, remainWidth, 0);

	bHandled = TRUE;
	return 1;
}

//--------------------------------------------------------------------------------------------
LRESULT TraceFrameWnd::OnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);
	if(width>0 && height>0 && m_wndListView.m_hWnd!=0 )
	{
		RECT rect;
		GetClientRect(&rect);

		int nWidth0 = m_wndListView.GetColumnWidth(0);
		int nWidth1 = m_wndListView.GetColumnWidth(1);

		m_wndListView.MoveWindow(0, 0, width, height);
		m_wndListView.SetColumnWidth(2, width-nWidth0-nWidth1-32);
	}
	bHandled = FALSE;
	return FALSE;
}

//--------------------------------------------------------------------------------------------
LRESULT TraceFrameWnd::OnSetFocus(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	System::getSingleton()->getMainFrame()->onChildActive(this);
	return 1;
}

//--------------------------------------------------------------------------------------------
void TraceFrameWnd::insertLog(unsigned int styleID, const AXIATRACE_TIME* tTime, const wchar_t* logString, unsigned int len_string)
{
	int nCount = m_wndListView.GetItemCount();
	
	wchar_t strIndex[32];
	_snwprintf(strIndex, 32, _T("%d"), m_nLogIndex++);
	m_wndListView.InsertItem(nCount, strIndex);

	wchar_t szTime[64];
	_snwprintf(szTime, 64, _T("%02d:%02d %02d.%03d"), tTime->wHour, tTime->wMinute, tTime->wSecond, tTime->wMilliseconds);

	m_wndListView.SetItemText(nCount, 1, szTime);

	//may be multi line
	const wchar_t* current=logString;
	do
	{
		const wchar_t* end = wcschr(current, L'\n');
		if(end==0)
		{
			if(m_wndListView.GetItemCount()!=nCount+1) 
			{
				m_wndListView.InsertItem(nCount, _T(""));
			}
			m_wndListView.SetItemText(nCount, 2, current);
			m_wndListView.SetItemData(nCount, (DWORD_PTR)styleID);
			break;
		}
		else
		{
			size_t lineChar = end-current;
			wchar_t* lineBuf = (wchar_t*)m_bufMaxLine.request((lineChar+1)*sizeof(wchar_t));
			memcpy(lineBuf, current, lineChar*sizeof(wchar_t));
			lineBuf[lineChar]=0;

			if(m_wndListView.GetItemCount()!=nCount+1) 
			{
				m_wndListView.InsertItem(nCount, _T(""));
			}
			m_wndListView.SetItemText(nCount, 2, lineBuf);
			m_wndListView.SetItemData(nCount, (DWORD_PTR)styleID);

			//next
			nCount++;
			current=end+1;
		}
	}while(true);

	if(System::getSingleton()->getConfig()->getAutoScroll())
	{
		m_wndListView.scrollToBottom();
	}
}

//--------------------------------------------------------------------------------------------
LRESULT TraceFrameWnd::OnClose(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	System::getSingleton()->getMainFrame()->onChildDestroy(this);
	bHandled=FALSE;
	return 1;
}

//--------------------------------------------------------------------------------------------
LRESULT TraceFrameWnd::OnFileSaveAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_wndListView.saveToFile();
	return 1;
}

}