/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once
#include "resource.h"

namespace AT3
{

/** About Dialog Class
*/
class AboutDlg : public CDialogImpl<AboutDlg>
{
public:
	enum { IDD = IDD_DIALOG_ABOUT };

	BEGIN_MSG_MAP(AboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_SYSLINK_HOME, NM_CLICK, OnSyslinkClick)
		NOTIFY_HANDLER(IDC_SYSLINK_HOME, NM_RETURN, OnSyslinkClick)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	/** Init Dialog
	*/
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/** On Close
	*/
	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnSyslinkClick(int id, LPNMHDR pNMHdr, BOOL& bHandled);
};

}