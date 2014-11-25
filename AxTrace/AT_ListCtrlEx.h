/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once

namespace AT3
{

/** List Control to draw trace text
*/
class CListCtrlEx : public CWindowImpl<CListCtrlEx, CListViewCtrl>, public CCustomDraw<CListCtrlEx>
{
public:
	BEGIN_MSG_MAP(CListCtrlEx)    
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)

		CHAIN_MSG_MAP(CCustomDraw<CListCtrlEx>)
	END_MSG_MAP()          	

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/);
	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW pCD);

public:
	enum { COLUMN_COUNTS=3, MAX_SUBITEM_LENGTH=2048 };	//hack...

	/** scroll to bottom */
	void scrollToBottom(void);
	/** copy select to clipboard */
	void copyToClipboard(void);
	/** save as a file*/
	void saveToFile(void);

private:
	COLORREF clrCell, clrCellBk, clrRow, clrRowBk, clrGrid, clrGridBk, clrBk;	

public:
	CListCtrlEx();
	~CListCtrlEx();
};

}