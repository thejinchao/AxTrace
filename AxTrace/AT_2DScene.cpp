/***************************************************

				AXIA|Trace3

			(C) Copyright  Jean. 2016
***************************************************/

#include "StdAfx.h"
#include "AT_2DScene.h"
#include "AT_2DFrame.h"
#include "AT_2DCamera.h"
#include "AT_Canvas.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
Graphics2DScene::Graphics2DScene(Graphics2DFrameWnd* frame)
	: m_frame(frame)
{

}

//--------------------------------------------------------------------------------------------
Graphics2DScene::~Graphics2DScene()
{

}

//--------------------------------------------------------------------------------------------
void Graphics2DScene::cleanScene(Real x_size, Real y_size)
{
	m_size.x = x_size;
	m_size.y = y_size;

	m_frame->getCamera()->setSceneSize(fVector2(x_size, y_size));
}

//--------------------------------------------------------------------------------------------
void Graphics2DScene::render(void)
{
	fVector2 sceneBorder[4] = { { 0.f, 0.f },{ m_size.x, 0.f },{ m_size.x, m_size.y },{ 0.f, m_size.y } };
	glColor3f(51.f / 255.f, 153.5F / 255.f, 255.f / 255.f);

	Canvas::drawPolygon(sceneBorder, 4);
}

}
