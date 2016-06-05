#include "StdAfx.h"
#include "AT_2DView.h"
#include "AT_2DFrame.h"
#include "AT_2DCamera.h"

namespace AT3
{

//-------------------------------------------------------------------------------------------------------------------
Graphics2DView::Graphics2DView(Graphics2DFrameWnd* frame)
	: m_frame(frame)
{
}

//-------------------------------------------------------------------------------------------------------------------
Graphics2DView::~Graphics2DView()
{

}

//-------------------------------------------------------------------------------------------------------------------
void Graphics2DView::OnGLInit(void)
{
	//init system
	RECT rect;
	GetClientRect(&rect);

	m_frame->initGL(rect.right - rect.left, rect.bottom - rect.top);
}

//-------------------------------------------------------------------------------------------------------------------
void Graphics2DView::OnGLRender(void)
{
	m_frame->render();
}

//-------------------------------------------------------------------------------------------------------------------
void Graphics2DView::OnGLResize(int width, int height)
{
	m_frame->getCamera()->setWindowSize(width, height);
}

//-------------------------------------------------------------------------------------------------------------------
LRESULT Graphics2DView::OnMouseMove(UINT key, const CPoint& point)
{
	//get mouse focus position
	_printMousePosition(point.x, point.y);

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------
LRESULT Graphics2DView::OnMouseLButtonDown(UINT key, const CPoint& point)
{

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------
LRESULT Graphics2DView::OnMouseLButtonUp(UINT key, const CPoint& point)
{

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------
LRESULT Graphics2DView::OnMouseMButtonDown(UINT key, const CPoint& point)
{

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------
LRESULT Graphics2DView::OnMouseMButtonUp(UINT key, const CPoint& point)
{

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------
LRESULT Graphics2DView::OnMouseWheel(UINT key, short zDelta, const CPoint& point)
{

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------
void Graphics2DView::_printMousePosition(int x, int y)
{
	fVector2 pos = m_frame->getCamera()->convertScreenToWorld(x, y);

	wchar_t wszTitleWithPos[MAX_PATH] = { 0 };
	StringCchPrintf(wszTitleWithPos, MAX_PATH, L"2D:%s (%.1f,%.1f)", 
		convertUTF8ToUTF16(m_frame->getWindowTitle().c_str(), m_frame->getWindowTitle().length()),
		pos.x, pos.y);

	::SetWindowTextW(m_frame->getNativeWnd(), wszTitleWithPos);
}

}
