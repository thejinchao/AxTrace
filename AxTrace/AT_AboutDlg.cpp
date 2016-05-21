/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#include "StdAfx.h"
#include "AT_AboutDlg.h"

namespace AT3
{



//--------------------------------------------------------------------------------------------
LRESULT AboutDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());
	return (LRESULT)TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT AboutDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT AboutDlg::OnSyslinkClick(int id, LPNMHDR pNMHdr, BOOL& bHandled)
{
	PNMLINK pNMLink = (PNMLINK)pNMHdr;
	LITEM item = pNMLink->item;

	// Judging by the index of the link
	if (item.iLink == 0) // If it is the first link
	{
		ShellExecute(NULL, L"open", item.szUrl, NULL, NULL, SW_SHOW);
	}
	return 0;

}

}