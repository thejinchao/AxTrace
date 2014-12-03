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
#include "AT_Message.h"

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
	m_wndListView.InsertColumn(2, _T("PID"), LVCFMT_LEFT, 50, 0);
	m_wndListView.InsertColumn(3, _T("TID"), LVCFMT_LEFT, 50, 0);

	int remainWidth = rect.right - rect.left - (40 + 90 + 50+ 50 + 32);
	m_wndListView.InsertColumn(4, _T("Log"), LVCFMT_LEFT, remainWidth, 0);

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

		int total = 0;
		for (int i = 0; i < 4; i++)
			total += m_wndListView.GetColumnWidth(i);

		m_wndListView.MoveWindow(0, 0, width, height);
		m_wndListView.SetColumnWidth(4, width - total - 32);
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
void TraceFrameWnd::insertLog(const LogMessage* message)
{
	const AXIATRACE_TIME* tTime = message->getTraceTime();
	unsigned int styleID = message->getStyleID();

	int nCount = m_wndListView.GetItemCount();
	
	wchar_t wszTemp[MAX_PATH];
	StringCchPrintfW(wszTemp, MAX_PATH, _T("%d"), m_nLogIndex++);
	m_wndListView.InsertItem(nCount, wszTemp);

	//Time
	StringCchPrintfW(wszTemp, MAX_PATH, _T("%02d:%02d %02d.%d"), tTime->wHour, tTime->wMinute, tTime->wSecond, tTime->wMilliseconds);
	m_wndListView.SetItemText(nCount, 1, wszTemp);

	//PID
	StringCchPrintfW(wszTemp, MAX_PATH, _T("%d"), message->getProcessID());
	m_wndListView.SetItemText(nCount, 2, wszTemp);

	//TID
	StringCchPrintfW(wszTemp, MAX_PATH, _T("%d"), message->getThreadID());
	m_wndListView.SetItemText(nCount, 3, wszTemp);

	//may be multi line
	const wchar_t* current = message->getLogBuf();
	do
	{
		const wchar_t* end = wcschr(current, L'\n');
		if(end==0)
		{
			if(m_wndListView.GetItemCount()!=nCount+1) 
			{
				m_wndListView.InsertItem(nCount, _T(""));
			}
			m_wndListView.SetItemText(nCount, 4, current);
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
			m_wndListView.SetItemText(nCount, 4, lineBuf);
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