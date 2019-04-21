/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_Camera2D.h"

//--------------------------------------------------------------------------------------------
Camera2D::Camera2D()
	: m_scale(1.0)
	, m_draging(false)
{

}

//--------------------------------------------------------------------------------------------
Camera2D::~Camera2D()
{

}

//--------------------------------------------------------------------------------------------
void Camera2D::_updateTransform(void)
{
	qreal screenRatio = (qreal)m_viewSize.width() / (qreal)m_viewSize.height();
	qreal sceneRatio = abs(m_sceneRect.width() / m_sceneRect.height());

	if (screenRatio > sceneRatio)
	{
		m_scale = m_viewSize.height() / abs(m_sceneRect.height()*1.2);
	}
	else
	{
		m_scale = m_viewSize.width() / abs(m_sceneRect.width()*1.2);
	}

	int flipX = m_sceneRect.width() > 0 ? 1 : -1;
	int flipY = m_sceneRect.height() > 0 ? 1 : -1;

	m_transform = QTransform::fromTranslate(-m_sceneRect.left(), -m_sceneRect.top());
	m_transform *= QTransform::fromScale(flipX*m_scale, flipY*m_scale);
	m_transform *= QTransform::fromTranslate((m_viewSize.width() - abs(m_sceneRect.width()) * m_scale) / 2, (m_viewSize.height() - abs(m_sceneRect.height()) * m_scale) / 2);
}

//--------------------------------------------------------------------------------------------
void Camera2D::reset(const QSize& viewSize, const QRectF& sceneRect)
{
	if (viewSize==m_viewSize && m_sceneRect == sceneRect) return;

	m_viewSize = viewSize;
	m_sceneRect = sceneRect;

	_updateTransform();
}

//--------------------------------------------------------------------------------------------
void Camera2D::updateViewSize(const QSize& viewSize)
{
	m_viewSize = viewSize;
}

//--------------------------------------------------------------------------------------------
QPointF Camera2D::screenToScene(const QPoint& pos)
{
	bool invertAble;
	QTransform inv = m_transform.inverted(&invertAble);
	if (invertAble) {
		QPointF scenePos = inv * QPointF(pos);
		return scenePos;
	}
	
	return QPointF(pos);
}

//--------------------------------------------------------------------------------------------
void Camera2D::onMouseWheel(QWheelEvent *e)
{
	qreal scale = 1.0;
	if (e->delta() > 0) {
		scale = 1.2;
	}
	else {
		scale = 0.9;
	}

	QPoint mousePoint = e->pos();
	m_transform *= QTransform::fromTranslate(-mousePoint.x(), -mousePoint.y());
	m_transform *= QTransform::fromScale(scale, scale);
	m_transform *= QTransform::fromTranslate(mousePoint.x(), mousePoint.y());

	m_scale *= scale;
}

//--------------------------------------------------------------------------------------------
void Camera2D::beginDrag(QMouseEvent *e)
{
	m_draging = true;
	m_beginDragPoint = e->pos();
	m_tragingTransform = m_transform;
}

//--------------------------------------------------------------------------------------------
void Camera2D::drag(QMouseEvent *e)
{
	if (!m_draging) return;

	QPoint move = e->pos() - m_beginDragPoint;
	m_transform = m_tragingTransform * QTransform::fromTranslate(move.x(), move.y());
}

//--------------------------------------------------------------------------------------------
void Camera2D::endDrag(QMouseEvent *e)
{
	m_draging = false;

	QPoint move = e->pos() - m_beginDragPoint;
	m_transform = m_tragingTransform * QTransform::fromTranslate(move.x(), move.y());
}
