#pragma once

#include "AtlOpengl.h"

namespace AT3
{

//pre-define
class Grphics2DMessage;
class Graphics2DFrameWnd;

/** OpenGL Canvas Window
*/
class Graphics2DView : public CWindowImpl<Graphics2DView>, COpenGL<Graphics2DView>
{
public:
	DECLARE_WND_CLASS_EX(NULL, 0, -1)

	void OnGLInit(void);
	void OnGLRender(void);
	void OnGLResize(int cx, int cy);

	BEGIN_MSG_MAP(Graphics2DView)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnMouseLButtonDown)
		MSG_WM_LBUTTONUP(OnMouseLButtonUp)
		MSG_WM_MBUTTONDOWN(OnMouseMButtonDown)
		MSG_WM_MBUTTONUP(OnMouseMButtonUp)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)

		CHAIN_MSG_MAP(COpenGL<Graphics2DView>)
	END_MSG_MAP()

	/******************************************************
		Message Handle
	*******************************************************/
	LRESULT OnMouseMove(UINT key, const CPoint& point);
	LRESULT OnMouseLButtonDown(UINT key, const CPoint& point);
	LRESULT OnMouseLButtonUp(UINT key, const CPoint& point);
	LRESULT OnMouseMButtonDown(UINT key, const CPoint& point);
	LRESULT OnMouseMButtonUp(UINT key, const CPoint& point);
	LRESULT OnMouseWheel(UINT key, short zDelta, const CPoint& point);

private:
	Graphics2DFrameWnd* m_frame;

	void _printMousePosition(int x, int y);

public:
	Graphics2DView(Graphics2DFrameWnd* frame);
	virtual ~Graphics2DView();
};

}
