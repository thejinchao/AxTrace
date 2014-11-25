/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#include "StdAfx.h"
#include "AT_OptionsDlg.h"

namespace AT3
{

//////////////////////////////////////////////////////////////////////////////////////////////
//Add Style Dialog
//////////////////////////////////////////////////////////////////////////////////////////////
class AddStyleDlg : public CDialogImpl<AddStyleDlg>
{
public:
	enum { IDD = IDD_DIALOG_ADDSTYLE };

	BEGIN_MSG_MAP(AddStyleDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOKCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancelCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CenterWindow(GetParent());
		SetDlgItemText(IDC_EDIT_STYLEID, _T("0"));
		::SetFocus(GetDlgItem(IDC_EDIT_STYLEID));

		SendDlgItemMessage(IDC_EDIT_STYLEID, EM_SETSEL, 0, -1);
		SendDlgItemMessage(IDC_EDIT_STYLEID, EM_SCROLLCARET, 0, 0L);
		//::edIT
		return FALSE;
	}
	LRESULT OnOKCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		int id = this->GetDlgItemInt(IDC_EDIT_STYLEID);
		if(id<=0 || id>=Config::MAX_TRACE_STYLE_COUNTS) return TRUE;
		
		m_styleID = id;

		EndDialog(wID);
		return TRUE;
	}
	LRESULT OnCancelCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		EndDialog(wID);
		return TRUE;
	}

	int getStyleID(void) const { return m_styleID; }

private:
	int m_styleID;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////
OptionsDlg::OptionsDlg(Config& config)
	: m_tempConfig(config)
	, m_curStyleID(-1)
{
}

//--------------------------------------------------------------------------------------------
OptionsDlg::~OptionsDlg()
{
}

//--------------------------------------------------------------------------------------------
LRESULT OptionsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());

	HWND hList = GetDlgItem(IDC_LIST_STYLE);
	m_styleList.Attach(hList);

	m_foreColor.SubclassWindow(GetDlgItem(IDC_BUTTON_FOREGROUND));
	m_foreColor.SetDefaultText(_T("Default Color"));
	m_foreColor.SetDefaultColor(GetSysColor(COLOR_WINDOWTEXT));

	m_backColor.SubclassWindow(GetDlgItem(IDC_BUTTON_BACKGROUND));
	m_backColor.SetDefaultText(_T("Default Color"));
	m_backColor.SetDefaultColor(GetSysColor(COLOR_WINDOW));

	m_preview.SubclassWindow(GetDlgItem(IDC_PREVIEW));

	RECT rect;
	::GetWindowRect(hList, &rect);
	m_styleList.InsertColumn(0, L"", LVCFMT_LEFT, rect.right-rect.left-32, 0);

	//insert default
	m_styleList.InsertItem(0, _T("Style0:Default"));

	//insert color type
	const Config::TraceStyle* allTraceStyle = m_tempConfig.getAllTraceStyle();
	for(int i=1; i<Config::MAX_TRACE_STYLE_COUNTS; i++)
	{
		const Config::TraceStyle& style = allTraceStyle[i];
		if(style.useDefault) continue;

		TCHAR szTemp[MAX_PATH]={0};
		_sntprintf(szTemp, MAX_PATH, _T("Style%d"), i);
		int index = m_styleList.InsertItem(m_styleList.GetItemCount(), szTemp);
		m_styleList.SetItemData(index, (DWORD_PTR)i);
	}
	m_curStyleID=0;
	m_styleList.SelectItem(0);
	return (LRESULT)TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT OptionsDlg::OnOKCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT OptionsDlg::OnCancelCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT OptionsDlg::OnButtonAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	AddStyleDlg dlg;
	if(IDOK!=dlg.DoModal()) return TRUE;
	int styleID = dlg.getStyleID();

	LVFINDINFO find={0};
	find.lParam= (UINT_PTR)styleID;
	find.flags = LVFI_PARAM;
	find.vkDirection = VK_DOWN;
	int index = m_styleList.FindItem(&find, 0);

	if(index<0)
	{
		//add new style
		m_tempConfig.addNewStyle(styleID);

		//m_tempConfig.add
		TCHAR szTemp[MAX_PATH]={0};
		_sntprintf(szTemp, MAX_PATH, _T("Style%d"), styleID);

		index = m_styleList.InsertItem(m_styleList.GetItemCount(), szTemp);
		m_styleList.SetItemData(index, (DWORD_PTR)styleID);
	}

	//select item
	m_styleList.SelectItem(index);

	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT OptionsDlg::OnButtonRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	LVFINDINFO find={0};
	find.lParam= (UINT_PTR)m_curStyleID;
	find.flags = LVFI_PARAM;
	find.vkDirection = VK_DOWN;
	int index = m_styleList.FindItem(&find, 0);

	if(index>0)
	{
		m_tempConfig.removeStyle(m_curStyleID);
		m_styleList.DeleteItem(index);
		m_styleList.SelectItem(0);
	}

	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT OptionsDlg::OnForegroundSelOK(int wParam, LPNMHDR lpNMHDR, BOOL& bHandled)
{
	m_tempConfig.setForegroundColor(m_curStyleID, m_foreColor.GetColor());
	m_preview.setFrontColor(m_foreColor.GetColor());
	m_preview.InvalidateRect(0);
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT OptionsDlg::OnBackroundSelOK(int wParam, LPNMHDR lpNMHDR, BOOL& bHandled)
{
	m_tempConfig.setBackgroundColor(m_curStyleID, m_backColor.GetColor());
	m_preview.setBackColor(m_backColor.GetColor());
	m_preview.InvalidateRect(0);
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT OptionsDlg::OnListViewItemChanged(int wParam, LPNMHDR lpNMHDR, BOOL& bHandled)
{
	LPNMLISTVIEW lpListView = (LPNMLISTVIEW)lpNMHDR;

	if( (lpListView->uChanged == LVIF_STATE) && 
		((lpListView->uNewState & LVIS_SELECTED)!=0))
	{
		m_curStyleID = (int)(DWORD_PTR)(lpListView->lParam);

		COLORREF colBack = m_tempConfig.getBackgroundColor(m_curStyleID);
		COLORREF colFront = m_tempConfig.getForegroundColor(m_curStyleID);

		m_foreColor.SetColor(colFront);
		m_backColor.SetColor(colBack);
		m_preview.setFrontColor(colFront);
		m_preview.setBackColor(colBack);
		m_preview.InvalidateRect(0);
	}
	return TRUE;
}

}