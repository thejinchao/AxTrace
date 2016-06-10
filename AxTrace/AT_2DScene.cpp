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
struct Graphics2DScene::Actor
{
	unsigned int id;
	fVector2 pos;
	Real dir;
};

//--------------------------------------------------------------------------------------------
Graphics2DScene::Graphics2DScene(Graphics2DFrameWnd* frame)
	: m_frame(frame)
	, m_active(false)
{

}

//--------------------------------------------------------------------------------------------
Graphics2DScene::~Graphics2DScene()
{

}

//--------------------------------------------------------------------------------------------
void Graphics2DScene::cleanScene(Real x_size, Real y_size, const fColor& borderColor, const fColor& backgroundColor)
{
	m_active = true;
	m_size.x = x_size;
	m_size.y = y_size;
	m_backgroundColor = backgroundColor;
	m_borderColor = borderColor;

	m_frame->getCamera()->setSceneSize(fVector2(x_size, y_size));
}

//--------------------------------------------------------------------------------------------
void Graphics2DScene::updateActor(unsigned int id, Real x, Real y, Real dir)
{
	Actor* actor;

	//is already exist?
	ActorMap::iterator it = m_actorMap.find(id);
	if (it != m_actorMap.end()) {
		actor = it->second;
	}
	else {
		//new 
		actor = new Actor;
		m_actorMap.insert(std::make_pair(id, actor));
	}

	actor->id = id;
	actor->pos.x = x;
	actor->pos.y = y;
	actor->dir = dir;
}

//--------------------------------------------------------------------------------------------
void Graphics2DScene::render(void)
{
	if (!m_active) return;


	//draw scene
	fVector2 sceneBorder[4] = { { 0.f, 0.f },{ m_size.x, 0.f },{ m_size.x, m_size.y },{ 0.f, m_size.y } };
	Canvas::drawSolidPolygon(sceneBorder, 4, m_backgroundColor);
	Canvas::drawPolygon(sceneBorder, 4, m_borderColor);


	//draw actor
	ActorMap::const_iterator it, _end = m_actorMap.end();

	for (it = m_actorMap.begin(); it != _end; ++it) {
		const Actor* actor = it->second;

		glColor3f(255.f / 255.f, 153.5F / 255.f, 255.f / 255.f);
		Canvas::drawSolidCircle(actor->pos, 1.f, fColor(1.0f, 0.0f, 0.0f, 1.0f));
	}

}



}
