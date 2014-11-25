/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_StylePreview.h"
#include "AT_System.h"
#include "AT_Config.h"

namespace AT3
{
//--------------------------------------------------------------------------------------------
StylePreview::StylePreview()
{

}

//--------------------------------------------------------------------------------------------
StylePreview::~StylePreview()
{

}

//--------------------------------------------------------------------------------------------
void StylePreview::setFrontColor(COLORREF colFront)
{
	m_colFront = colFront;
}

//--------------------------------------------------------------------------------------------
void StylePreview::setBackColor(COLORREF colBack)
{
	m_colBack = colBack;
}

//--------------------------------------------------------------------------------------------
LRESULT StylePreview::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TCHAR* szSample = _T("Text Sample");
	int lenSample = _tcslen(szSample);

	PAINTSTRUCT ps;
	BeginPaint(&ps);

	HFONT oldFont = (HFONT)SelectObject(ps.hdc, System::getSingleton()->getConfig()->getFont());

	HBRUSH hBackGroundBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	FillRect(ps.hdc, &(ps.rcPaint), hBackGroundBrush);

	RECT rect={0};
	DrawText(ps.hdc, szSample, lenSample, &rect, DT_CALCRECT);

	RECT rectCanvas;
	GetClientRect(&rectCanvas);

	int nTop = (rectCanvas.bottom-rect.bottom)/2;
	SetRect(&rect, 0, nTop, rectCanvas.right, nTop+rect.bottom);
	
	HBRUSH hBackBrush = ::CreateSolidBrush(m_colBack);
	FillRect(ps.hdc, &rect, hBackBrush);

	SetTextColor(ps.hdc, m_colFront);
	SetBkMode(ps.hdc, TRANSPARENT);
	DrawText(ps.hdc, szSample, lenSample, &rect, DT_CENTER); 

	DeleteObject(hBackBrush);
	DeleteObject(hBackGroundBrush);
	SelectObject(ps.hdc, oldFont);
	EndPaint(&ps);
	bHandled = TRUE;
	return 0;
}

}
