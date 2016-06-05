#include "stdafx.h"
#include "AT_2DCamera.h"

namespace AT3
{ 

//-------------------------------------------------------------------------------------------------------------------
Graphics2DCamera::Graphics2DCamera(onCameraAdjust callBack, void* param)
	: m_callBack(callBack)
	, m_param(param)
	, m_viewZoom(1.f)
	, m_viewCenter(1.f, 0.f)
{
}

//-------------------------------------------------------------------------------------------------------------------
Graphics2DCamera::~Graphics2DCamera()
{

}

//-------------------------------------------------------------------------------------------------------------------
void Graphics2DCamera::_update(void)
{
	Real ratio = (Real)m_winSize.x / (Real)m_winSize.y;

	fVector2 extents(ratio*(Real)DEFAULT_HALF_HEIGHT, (Real)DEFAULT_HALF_HEIGHT);
	extents /= m_viewZoom;

	fVector2 lower = m_viewCenter - extents;
	fVector2 upper = m_viewCenter + extents;

	if (m_callBack)
	{
		m_callBack(m_winSize.x, m_winSize.y, lower, upper, m_param);
	}
}


//-------------------------------------------------------------------------------------------------------------------
fVector2 Graphics2DCamera::convertScreenToWorld(int x, int y)
{
	Real u = x / float(m_winSize.x);
	Real v = (m_winSize.y - y) / Real(m_winSize.y);

	Real ratio = (Real)m_winSize.x / (Real)m_winSize.y;

	fVector2 extents(ratio*(Real)DEFAULT_HALF_HEIGHT, (Real)DEFAULT_HALF_HEIGHT);
	extents /= m_viewZoom;

	fVector2 lower = m_viewCenter - extents;
	fVector2 upper = m_viewCenter + extents;

	fVector2 p;

	p.x = (1.0f - u) * lower.x + u * upper.x;
	p.y = (1.0f - v) * lower.y + v * upper.y;

	return p;
}

//-------------------------------------------------------------------------------------------------------------------
Real Graphics2DCamera::convertScreenSizeToWorld(Real size)
{
	return size*(DEFAULT_HALF_HEIGHT / m_viewZoom) / (m_winSize.y / 2);
}

//-------------------------------------------------------------------------------------------------------------------
void Graphics2DCamera::setWindowSize(int width, int height)
{
	m_winSize.x = width;
	m_winSize.y = height;

	_update();
}

//-------------------------------------------------------------------------------------------------------------------
void Graphics2DCamera::setViewZoom(Real viewZoom)
{
	if (viewZoom>20.0 || viewZoom<0.01) return;

	m_viewZoom = viewZoom;

	_update();
}

//-------------------------------------------------------------------------------------------------------------------
void Graphics2DCamera::pan(const fVector2& offset)
{
	m_viewCenter -= offset;

	_update();
}

//-------------------------------------------------------------------------------------------------------------------
void Graphics2DCamera::setSceneSize(const fVector2& sceneSize)
{
	const float SCREEN_SCALE = 1.2f;

	m_viewCenter = 0.5f * sceneSize;

	Real screenRatio = (Real)m_winSize.x / (Real)m_winSize.y;
	Real sceneRatio = sceneSize.x / sceneSize.y;

	if (sceneRatio > screenRatio) {
		//use width
		m_viewZoom = DEFAULT_HALF_HEIGHT*screenRatio / (sceneSize.x*SCREEN_SCALE / 2.f);
	}
	else {
		//use height
		m_viewZoom = DEFAULT_HALF_HEIGHT / (sceneSize.y*SCREEN_SCALE / 2.f);
	}

	_update();
}

}
