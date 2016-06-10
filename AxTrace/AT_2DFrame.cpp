/***************************************************

				AXIA|Trace3

		(C) Copyright  Jean. 2016
***************************************************/

#include "StdAfx.h"
#include "AT_2DFrame.h"
#include "AT_Util.h"
#include "AT_System.h"
#include "AT_MainFrame.h"
#include "AT_Config.h"
#include "AT_Message.h"
#include "AT_2DCamera.h"
#include "AT_2DScene.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
Graphics2DFrameWnd::Graphics2DFrameWnd(CUpdateUIBase* pUpdateUI, const std::string& windowTitle)
	: m_pUpdateUI(pUpdateUI)
	, m_windowTitle(windowTitle)
	, m_wndCanvas(this)
{
}

//--------------------------------------------------------------------------------------------
Graphics2DFrameWnd::~Graphics2DFrameWnd()
{

}

//--------------------------------------------------------------------------------------------
void Graphics2DFrameWnd::initGL(int screen_x, int screen_y)
{
	m_camera = new Graphics2DCamera(_onCameraAdjust, this);
	m_scene = new Graphics2DScene(this);

	//init opengl status
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClearDepth(1.0f);									// Depth Buffer Setup

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
}

//--------------------------------------------------------------------------------------------
void Graphics2DFrameWnd::render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	fVector2 lowPos = m_lowPos;
	fVector2 upperPos = m_upperPos;
	gluOrtho2D(lowPos.x, upperPos.x, lowPos.y, upperPos.y);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
														
	
	m_scene->render();


	glFlush();
}

//-------------------------------------------------------------------------------------------------------------------
void Graphics2DFrameWnd::_onCameraAdjust(int sreenWidth, int screenHeight, const fVector2& lowPos, const fVector2& upperPos, void* param)
{
	Graphics2DFrameWnd* pThis = (Graphics2DFrameWnd*)param;

	glViewport(0, 0, sreenWidth, screenHeight);

	pThis->m_lowPos = lowPos;
	pThis->m_upperPos = upperPos;
}

//--------------------------------------------------------------------------------------------
void Graphics2DFrameWnd::redraw(void)
{
	m_wndCanvas.InvalidateRect(0);
}

//--------------------------------------------------------------------------------------------
LRESULT Graphics2DFrameWnd::OnEditClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	return TRUE;
}

//--------------------------------------------------------------------------------------------
LRESULT Graphics2DFrameWnd::OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	m_hWndClient = m_wndCanvas.Create(m_hWnd, rcDefault, NULL, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CS_HREDRAW | CS_VREDRAW | CS_OWNDC, 
		0);

	bHandled = TRUE;
	return 1;
}

//--------------------------------------------------------------------------------------------
LRESULT Graphics2DFrameWnd::OnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);
	if (width > 0 && height > 0 && m_wndCanvas.m_hWnd != 0)
	{
		m_wndCanvas.MoveWindow(0, 0, width, height);
	}
	bHandled = FALSE;
	return FALSE;
}

//--------------------------------------------------------------------------------------------
LRESULT Graphics2DFrameWnd::OnSetFocus(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	System::getSingleton()->getMainFrame()->onChildActive(this);
	return 1;
}

//--------------------------------------------------------------------------------------------
LRESULT Graphics2DFrameWnd::OnClose(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	System::getSingleton()->getMainFrame()->onChildDestroy(this);
	bHandled = FALSE;
	return 1;
}

//--------------------------------------------------------------------------------------------
void Graphics2DFrameWnd::cleanMap(const G2DCleanMapMessage* message, const Filter::Result& filter)
{
	double x_size = message->get_x_size();
	double y_size = message->get_y_size();
	fColor borderColor(fColor::convert_RGB555_to_RGB888(filter.fontColor));
	fColor backgroundColor(fColor::convert_RGB555_to_RGB888(filter.backColor));

	m_scene->cleanScene((Real)x_size, (Real)y_size, borderColor, backgroundColor);
}

//--------------------------------------------------------------------------------------------
void Graphics2DFrameWnd::updateActor(const G2DActorMessage* message)
{
	m_scene->updateActor(message->get_id(), message->get_x(), message->get_y(), message->get_dir());
}


}