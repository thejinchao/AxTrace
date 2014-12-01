/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_ListCtrlEx.h"
#include "AT_Config.h"
#include "AT_System.h"

namespace AT3
{
//--------------------------------------------------------------------------------------------
CListCtrlEx::CListCtrlEx()
{

}
//--------------------------------------------------------------------------------------------
CListCtrlEx::~CListCtrlEx()
{

}

//--------------------------------------------------------------------------------------------
LRESULT CListCtrlEx::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// Colors
	clrBk = ::GetSysColor(COLOR_WINDOW);

	clrCellBk = RGB(0x00,0xFF,0x00);
	clrCell   = RGB(0xff,0,0);

	clrGridBk = RGB(0x00,0x00,0xFF);
	clrGrid   = RGB(0x7f,0,0);

	bHandled = FALSE;
	return 0;
}

//--------------------------------------------------------------------------------------------
LRESULT CListCtrlEx::OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	SetExtendedListViewStyle(LVS_EX_FULLROWSELECT); 
	SetFont(System::getSingleton()->getConfig()->getFont());
	SetBkColor(clrBk);

	bHandled = FALSE;
	return 0;
}

//--------------------------------------------------------------------------------------------
DWORD CListCtrlEx::OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/) 
{ 
	return CDRF_NOTIFYITEMDRAW;
}

//--------------------------------------------------------------------------------------------
DWORD CListCtrlEx::OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW pCD)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pCD);

	const Config* config = System::getSingleton()->getConfig();

	int styleID = pCD->lItemlParam;
	pLVCD->clrText = config->getForegroundColor(styleID);
	pLVCD->clrTextBk = config->getBackgroundColor(styleID);
	return CDRF_DODEFAULT;
} 

//--------------------------------------------------------------------------------------------
void CListCtrlEx::scrollToBottom(void)
{
	int n = GetItemCount();
	if(n>0)
	{
		EnsureVisible(n-1, FALSE);
	}
}

//--------------------------------------------------------------------------------------------
void CListCtrlEx::copyToClipboard(void)
{
	int nItem = (int)GetNextItem(-1, LVIS_SELECTED);

	if(nItem<0) return;
	if(!OpenClipboard()) return;

	std::wstring strClip;
	wchar_t wszTemp[MAX_SUBITEM_LENGTH]={0};
	while(nItem>=0)
	{
		for(int i=0; i<COLUMN_COUNTS; i++)
		{
			GetItemText(nItem, i, wszTemp, MAX_SUBITEM_LENGTH);

			strClip += wszTemp;
			if(i != COLUMN_COUNTS-1) strClip += _T("\t");
			else strClip += _T("\r\n");
		}

		nItem = GetNextItem(nItem, LVIS_SELECTED);
	};

	size_t size = strClip.size();

	EmptyClipboard();
	HGLOBAL hCopy = GlobalAlloc(GMEM_MOVEABLE, (size+1)*sizeof(wchar_t));
	wchar_t* lpstrCopy = (wchar_t*)GlobalLock(hCopy);
	memcpy(lpstrCopy, strClip.c_str(), size*sizeof(wchar_t));
	lpstrCopy[size] = 0;
	GlobalUnlock(hCopy);

	SetClipboardData(CF_UNICODETEXT, hCopy);

	CloseClipboard();
	GlobalFree(hCopy);
}

//--------------------------------------------------------------------------------------------
void CListCtrlEx::saveToFile(void)
{
	CFileDialog dlg(FALSE, _T("Log file(*.log)"), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, _T("Log file(*.log)\0*.log\0All file(*.*)\0*.*\0\0"));
	if(IDOK!=dlg.DoModal()) return;

	TCHAR wszFileName[MAX_PATH]={0};
	StringCchCopyW(wszFileName, MAX_PATH, dlg.m_szFileName);
	if(PathFindExtension(wszFileName)==0)
	{
		PathRenameExtension(wszFileName, _T(".log"));
	}

	FILE* fp = 0;
	_wfopen_s(&fp, wszFileName, _T("wb"));
	if(fp==0) 
	{
		//Error!
		MessageBox(_T("Write log file error!"), _T("Axtrace3"), MB_OK|MB_ICONSTOP);
		return;
	}

	unsigned short UNICODE_HEAD = 0xFEFF;
	fwrite(&UNICODE_HEAD, 2, 1, fp);

	wchar_t wszTemp[MAX_SUBITEM_LENGTH]={0};

	int counts = this->GetItemCount();
	for(int i=0; i<counts; i++)
	{
		for(int j=0; j<COLUMN_COUNTS; j++)
		{
			GetItemText(i, j, wszTemp, MAX_SUBITEM_LENGTH);

			fwrite(wszTemp, 2, wcslen(wszTemp), fp);
			if(j != COLUMN_COUNTS-1) fwrite(_T("\t"), 2, 1, fp);
			else fwrite(_T("\r\n"), 2, 2, fp);
		}
	}

	fclose(fp);

}

}
