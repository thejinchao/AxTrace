/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#pragma once

class Camera2D
{
public:
	void reset(const QSize& viewSize, const QRectF& sceneRect);

	void updateViewSize(const QSize& viewSize);
	const QSize& getViewSize(void) const { return m_viewSize; }

	const QRectF& getSceneRect(void) const { return m_sceneRect; }

	QPointF screenToScene(const QPoint& pos);

	qreal getScale(void) const { return m_scale; }
	const QTransform& getTransform(void) const { return m_transform; }

	void onMouseWheel(QWheelEvent *e);
	void beginDrag(QMouseEvent *e);
	void drag(QMouseEvent *e);
	void endDrag(QMouseEvent *e);
	bool isDraging(void) const { return m_draging; }

private:
	void _updateTransform(void);

private:
	QSize m_viewSize;
	QRectF m_sceneRect;

	qreal m_scale;
	QTransform m_transform;

	bool m_draging;
	QPoint m_beginDragPoint;
	QTransform m_tragingTransform;

public:
	Camera2D();
	~Camera2D();
};
