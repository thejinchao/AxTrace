/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once


namespace AT3
{

/** List Control to draw trace text
*/
class StylePreview : public CWindowImpl<StylePreview, CStatic>
{
public:
	BEGIN_MSG_MAP(StylePreview)    
		MESSAGE_HANDLER(WM_PAINT, OnPaint)

	END_MSG_MAP()          	

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void setFrontColor(COLORREF colFront);
	void setBackColor(COLORREF colBack);

private:
	HBRUSH m_hBackGroundBrush;
	COLORREF m_colFront;
	COLORREF m_colBack;

public:
	StylePreview();
	~StylePreview();
};

}
