#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QBrush>
#include <QFont>
#include <QPen>
#include "AT4_Scene2D.h"

#include "AT4_Filter.h"

class Camera2D;
class Begin2DSceneMessage;
class Update2DActorMessage;
class End2DSceneMessage;

class Map2DChild : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	void init(QWidget* parent);
	void clean(void);
	void beginScene(Begin2DSceneMessage* msg);
	void updateActor(Update2DActorMessage* msg, const Filter::Actor2DResult& filterResult);
	void endScene(End2DSceneMessage* msg);

protected:
	void closeEvent(QCloseEvent *event) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;

	void paintEvent(QPaintEvent *event) override;
	void resizeGL(int w, int h) override;

private:
	QString m_title;

	QBrush m_backgroundBrush;
	QBrush m_sceneBrush;
	QPen m_sceneBorderPen;
	QPen m_sceneGridPen;
	QPen m_infoTextPen;
	QFont m_infoTextFont;

	Scene2D* m_scene;
	Camera2D* m_camera;

	QPointF	m_cursorPosView;
	QPointF m_cursorPosScene;

private:
	void _getMouseTips(const QTransform& localMove, const Scene2D::Actor& actor, QString& mouseTips);
	void _drawGrid(QPainter& painter);

public:
	static void initCachedObject(void);
	static void deleteCachedObject(void);

private:
	static QPen& getCachedPen(uint16_t color);
	static QBrush& getCachedBrush(uint16_t color);

	enum { MAX_COLOR_COUNTS = 0xFFF };
	static QPen*	m_cachedPen[MAX_COLOR_COUNTS];
	static QBrush*	m_cachedBrush[MAX_COLOR_COUNTS];

public:
	Map2DChild(const QString& title);
	virtual ~Map2DChild();
};

