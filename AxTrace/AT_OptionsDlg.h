/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once
#include "resource.h"
#include "AT_Config.h"
#include "AT_ColorButton.h"
#include "AT_StylePreview.h"

namespace AT3
{

/** About Dialog Class
*/
class OptionsDlg : public CDialogImpl<OptionsDlg>
{
public:
	enum { IDD = IDD_DIALOG_SETTING };

	BEGIN_MSG_MAP(OptionsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOKCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancelCmd)
		COMMAND_ID_HANDLER(IDC_BUTTON_ADD, OnButtonAdd)
		COMMAND_ID_HANDLER(IDC_BUTTON_REMOVE, OnButtonRemove)
		NOTIFY_HANDLER(IDC_LIST_STYLE, LVN_ITEMCHANGED, OnListViewItemChanged)
		NOTIFY_HANDLER(IDC_BUTTON_FOREGROUND, CPN_SELENDOK, OnForegroundSelOK)
		NOTIFY_HANDLER(IDC_BUTTON_BACKGROUND, CPN_SELENDOK, OnBackroundSelOK)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	/** Init Dialog
	*/
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnOKCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancelCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnButtonAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnButtonRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnListViewItemChanged(int wParam, LPNMHDR lpNMHDR, BOOL& bHandled);
	LRESULT OnForegroundSelOK(int wParam, LPNMHDR lpNMHDR, BOOL& bHandled);
	LRESULT OnBackroundSelOK(int wParam, LPNMHDR lpNMHDR, BOOL& bHandled);

	const Config& getSetting(void) const { return m_tempConfig; }

private:
	Config			m_tempConfig;
	CListViewCtrl	m_styleList;
	CColorButton	m_foreColor;
	CColorButton	m_backColor;
	int				m_curStyleID;
	StylePreview	m_preview;

public:
	OptionsDlg(Config& config);
	virtual ~OptionsDlg();
};

}