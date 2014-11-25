/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_ValueFrame.h"
#include "AT_Util.h"
#include "AT_System.h"
#include "AT_MainFrame.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
ValueFrameWnd::ValueFrameWnd(CUpdateUIBase* pUpdateUI, int windowID)
	: m_pUpdateUI(pUpdateUI)
	, m_windowID(windowID)
{

}

//--------------------------------------------------------------------------------------------
ValueFrameWnd::~ValueFrameWnd()
{

}

//--------------------------------------------------------------------------------------------
void ValueFrameWnd::redraw(void)
{
	m_wndListView.InvalidateRect(0);
}

//--------------------------------------------------------------------------------------------
LRESULT ValueFrameWnd::OnEditCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_wndListView.copyToClipboard();
	return TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT ValueFrameWnd::OnEditClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_wndListView.DeleteAllItems();
	m_valueHashMap.clear();
	return TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT ValueFrameWnd::OnEditSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int count=m_wndListView.GetItemCount();
	for(int i=0; i<count; i++)
	{
		m_wndListView.SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	return TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT ValueFrameWnd::OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	m_hWndClient = m_wndListView.Create(m_hWnd, rcDefault, NULL, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		LVS_REPORT | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER,
		WS_EX_CLIENTEDGE);

	RECT rect;
	GetClientRect(&rect);

	// Create list view columns
	m_wndListView.InsertColumn(0, _T("Time"), LVCFMT_RIGHT, 90, 1);
	m_wndListView.InsertColumn(1, _T("Name"), LVCFMT_LEFT, 200, 0);

	int remainWidth = rect.right-rect.left-(200+90+32);
	m_wndListView.InsertColumn(2, _T("Value"), LVCFMT_LEFT, 100, 2);

	bHandled = FALSE;
	return 1;
}

//--------------------------------------------------------------------------------------------
LRESULT ValueFrameWnd::OnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);
	if(width>0 && height>0 && m_wndListView.m_hWnd!=0 )
	{
		RECT rect;
		GetClientRect(&rect);

		int nWidth0 = m_wndListView.GetColumnWidth(0);
		int nWidth1 = m_wndListView.GetColumnWidth(1);

		m_wndListView.SetColumnWidth(2, width-nWidth0-nWidth1-32);

	}
	bHandled = FALSE;
	return 1;
}

//--------------------------------------------------------------------------------------------
LRESULT ValueFrameWnd::OnSetFocus(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	System::getSingleton()->getMainFrame()->onChildActive(this);
	return 1;
}

//--------------------------------------------------------------------------------------------
void ValueFrameWnd::watchValue(unsigned int styleID, const AXIATRACE_TIME* tTime, const wchar_t* valueName, const wchar_t* value)
{
	int nIndex = 0;

	wchar_t szTime[64];
	_snwprintf(szTime, 64, _T("%02d:%02d %02d.%03d"), tTime->wHour, tTime->wMinute, tTime->wSecond, tTime->wMilliseconds);

	//find in hash map
	ValueHashMap::iterator it = m_valueHashMap.find(valueName);
	if(it==m_valueHashMap.end())
	{
		nIndex = m_wndListView.GetItemCount();
		m_valueHashMap.insert(std::make_pair(valueName, nIndex));

		m_wndListView.InsertItem(nIndex, szTime);
		m_wndListView.SetItemText(nIndex, 1, valueName);
	}
	else
	{
		nIndex = it->second;
		m_wndListView.SetItemText(nIndex, 0, szTime);
	}

	m_wndListView.SetItemText(nIndex, 2, value);
	m_wndListView.SetItemData(nIndex, (DWORD_PTR)styleID);
}

//--------------------------------------------------------------------------------------------
LRESULT ValueFrameWnd::OnClose(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	System::getSingleton()->getMainFrame()->onChildDestroy(this);
	bHandled = FALSE;
	return 1;
}

//--------------------------------------------------------------------------------------------
LRESULT ValueFrameWnd::OnFileSaveAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_wndListView.saveToFile();
	return 1;
}


}