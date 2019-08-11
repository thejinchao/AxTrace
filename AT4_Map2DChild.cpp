/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"

#include "AT4_Map2DChild.h"
#include "AT4_ChildInterface.h"
#include "AT4_Scene2D.h"
#include "AT4_Camera2D.h"
#include "AT4_System.h"
#include "AT4_MainWindow.h"
#include "AT4_Message.h"
#include "AT4_System.h"
#include "AT4_Config.h"

//--------------------------------------------------------------------------------------------
class Map2DChildInterface : public IChild
{
public:
	virtual Type getType(void) const { return CT_2DMAP; }

	virtual QString getTitle(void) const {
		return m_proxy->windowTitle();
	}

	virtual bool copyAble(void) const {
		return false;
	}

	virtual void onCopy(void) const {

	}

	virtual void clean(void) 
	{
		m_proxy->clean();
	}

	virtual void saveAs(void) {

	}

	virtual void update(void)
	{
		m_proxy->update();
	}

private:
	Map2DChild* m_proxy;

public:
	Map2DChildInterface(Map2DChild* proxy) : m_proxy(proxy) { }
	~Map2DChildInterface() {}
};

//--------------------------------------------------------------------------------------------
QPen* Map2DChild::m_cachedPen[Map2DChild::MAX_COLOR_COUNTS] = { nullptr };
QBrush*	Map2DChild::m_cachedBrush[Map2DChild::MAX_COLOR_COUNTS] = { nullptr };

//--------------------------------------------------------------------------------------------
Map2DChild::Map2DChild(const QString& title)
	: m_scene(nullptr)
	, m_camera(nullptr)
{
	setAttribute(Qt::WA_DeleteOnClose);

	m_title = title;
	QString windowTitle = tr("2DMap:%1").arg(title);
	setWindowTitle(windowTitle);
	setAutoFillBackground(false);

	m_camera = new Camera2D();
	this->setUserData(0, new Map2DChildInterface(this));
}

//--------------------------------------------------------------------------------------------
Map2DChild::~Map2DChild()
{
	Map2DChildInterface* i = (Map2DChildInterface*)(this->userData(0));
	delete i;

	this->setUserData(0, nullptr);
	delete m_camera;
	delete m_scene;
}

//--------------------------------------------------------------------------------------------
void Map2DChild::init(QWidget* parent)
{
	this->setParent(parent);

	m_backgroundBrush = QBrush(QColor(83, 83, 83));
	m_sceneBrush = QBrush(QColor(77, 128, 244));
	m_sceneBorderPen = QPen(Qt::white);
	m_sceneGridPen = QPen(Qt::gray);
	m_infoTextPen = QPen(Qt::white);
	m_infoTextFont.setPixelSize(16);
	setMouseTracking(true);
}

//--------------------------------------------------------------------------------------------
void Map2DChild::clean(void)
{
	if (m_scene)
	{
		m_scene->clean();
		update();
	}
}

//--------------------------------------------------------------------------------------------
void Map2DChild::beginScene(Begin2DSceneMessage* msg)
{
	if (!m_scene)
	{
		m_scene = new Scene2D(msg);
		m_camera->reset(size(), m_scene->getSceneRect());
	}

	m_scene->beginScene(msg);
}

//--------------------------------------------------------------------------------------------
void Map2DChild::updateActor(Update2DActorMessage* msg, const Filter::Actor2DResult& filterResult)
{
	if (m_scene)
	{
		m_scene->updateActor(msg, filterResult);
	}
}

//--------------------------------------------------------------------------------------------
void Map2DChild::endScene(End2DSceneMessage* msg)
{
	if (m_scene) 
	{
		m_scene->endScene(msg);
		m_camera->reset(size(), m_scene->getSceneRect());
	}

	update();
}

//--------------------------------------------------------------------------------------------
void Map2DChild::closeEvent(QCloseEvent *event)
{
	System::getSingleton()->getMainWindow()->notifySubWindowClose(IChild::CT_2DMAP, m_title);
	event->accept();
}

//--------------------------------------------------------------------------------------------
void Map2DChild::mousePressEvent(QMouseEvent *e)
{
	if (!m_camera) return;

	if (e->button() == Qt::RightButton) {
		grabMouse();
		m_camera->beginDrag(e);
	}
}

//--------------------------------------------------------------------------------------------
void Map2DChild::mouseMoveEvent(QMouseEvent *e)
{
	if (!m_camera) return;

	m_cursorPosView = QPointF(e->pos());
	m_cursorPosScene = m_camera->screenToScene(e->pos());
	if (m_camera->isDraging()) {
		m_camera->drag(e);
	}
	
	update();
}
//--------------------------------------------------------------------------------------------
void Map2DChild::mouseReleaseEvent(QMouseEvent *e)
{
	if (!m_camera) return;

	if (e->button() == Qt::RightButton) {
		releaseMouse();
		m_camera->endDrag(e);
	}
}

//--------------------------------------------------------------------------------------------
void Map2DChild::wheelEvent(QWheelEvent *e)
{
	if (!m_camera) return;

	m_camera->onMouseWheel(e);
	update();
}

//--------------------------------------------------------------------------------------------
void Map2DChild::resizeGL(int w, int h) 
{
	if (!m_camera) return;

	m_camera->updateViewSize(QSize(w, h));
}

//--------------------------------------------------------------------------------------------
void Map2DChild::paintEvent(QPaintEvent *event)
{
	QPainter painter;
	painter.begin(this);
	painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

	painter.fillRect(event->rect(), m_backgroundBrush);
	QString mouseTips;

	painter.save();
	if (m_scene)
	{
		painter.setTransform(m_camera->getTransform());

		const QRectF& sceneRect = m_scene->getSceneRect();

		//scene background
		m_sceneBorderPen.setWidthF(1.0 / m_camera->getScale());
		painter.setPen(m_sceneBorderPen);
		painter.fillRect(m_scene->getSceneRect(), m_sceneBrush);

		//scene grid
		if (m_scene->isGridDefined() && System::getSingleton()->getConfig()->getShowGrid())
		{
			m_sceneGridPen.setWidthF(1.0 / m_camera->getScale());
			painter.setPen(m_sceneGridPen);

			_drawGrid(painter);
		}

		Filter* filter = System::getSingleton()->getFilter();

		//actor
		m_scene->walk([&](const Scene2D::Actor& actor) 
		{
			painter.setBrush(getCachedBrush(actor.fillColor));

			QPen& actorPen = getCachedPen(actor.borderColor);
			actorPen.setWidthF(1.0 / m_camera->getScale());
			painter.setPen(actorPen);

			painter.save();

			QTransform localMove = QTransform::fromTranslate(actor.pos.x(), actor.pos.y());
			localMove.rotateRadians(actor.dir);
			painter.setTransform(localMove, true);

			_getMouseTips(painter.transform(), actor, mouseTips);

			switch (actor.type)
			{
			case Filter::AT_CIRCLE:
			{
				painter.drawEllipse(QPointF(0.0, 0.0), actor.size, actor.size);
				painter.drawLine(QPointF(0.0, 0.0), QPointF(actor.size*1.2, 0.0));
			}
			break;

			case Filter::AT_QUAD:
			{
				painter.drawRect(QRectF(-actor.size, -actor.size, actor.size*2, actor.size*2));
				painter.drawLine(QPointF(0.0, 0.0), QPointF(actor.size*1.2, 0.0));
			}
			break;

			case Filter::AT_TRIANGLE:
			{
				float l = -0.866025*actor.size; // sqrt(0.75)
				float t = 0.5*actor.size;
				QPointF triangles[3] = { 
					QPointF(actor.size, 0),
					QPointF(l, -t),
					QPointF(l,  t)
				};
				painter.drawConvexPolygon(triangles, 3);
			}
			break;
			}

			painter.restore();
		});
	}
	painter.restore();

	if (m_scene)
	{
		painter.setPen(m_infoTextPen);
		painter.setFont(m_infoTextFont);

		QString infoText = QString("SceneSize:%1,%2\nMouse:%3,%4\n%5")
			.arg(abs(m_scene->getSceneRect().width()))
			.arg(abs(m_scene->getSceneRect().height()))
			.arg(m_cursorPosScene.x())
			.arg(m_cursorPosScene.y())
			.arg(mouseTips);
		painter.drawText(QRect(0, 0, m_camera->getViewSize().width(), m_camera->getViewSize().height()), Qt::AlignLeft|Qt::AlignTop, infoText);
	}

	painter.end();
}

//--------------------------------------------------------------------------------------------
void Map2DChild::_drawGrid(QPainter& painter)
{
	const QRectF& sceneRect = m_scene->getSceneRect();
	const QSizeF& gridSize = m_scene->getGridSize();

	//draw v line
	double minX = qMin(sceneRect.left(), sceneRect.left() + sceneRect.width());
	double maxX = qMax(sceneRect.left(), sceneRect.left() + sceneRect.width());

	for (double x = m_scene->getGridPoint().x(); x > minX; x -= gridSize.width())
	{
		if (x < maxX) {
			painter.drawLine(QPointF(x, sceneRect.bottom()), QPointF(x, sceneRect.top()));
		}
	}

	for (double x = m_scene->getGridPoint().x() + gridSize.width(); x < maxX; x += gridSize.width())
	{
		if (x > minX) {
			painter.drawLine(QPointF(x, sceneRect.bottom()), QPointF(x, sceneRect.top()));
		}
	}

	//draw h line
	double minY = qMin(sceneRect.top(), sceneRect.top()+sceneRect.height());
	double maxY = qMax(sceneRect.top(), sceneRect.top() + sceneRect.height());

	for (double y = m_scene->getGridPoint().y(); y > minY; y-=gridSize.height())
	{
		if (y < maxY) {
			painter.drawLine(QPointF(sceneRect.left(), y), QPointF(sceneRect.right(), y));
		}
	}

	for (double y = m_scene->getGridPoint().y() + gridSize.height(); y < maxY; y += gridSize.height())
	{
		if (y > minY)
		{
			painter.drawLine(QPointF(sceneRect.left(), y), QPointF(sceneRect.right(), y));
		}
	}
}

//--------------------------------------------------------------------------------------------
void Map2DChild::_getMouseTips(const QTransform& localMove, const Scene2D::Actor& actor, QString& mouseTips)
{
	//calc cursor pos
	bool invertible;
	QTransform localMoveInvert = localMove.inverted(&invertible);
	if (!invertible) return;
	
	QPointF pos = localMoveInvert * m_cursorPosView;

	switch (actor.type)
	{
	case Filter::AT_CIRCLE:
	{
		if (QPointF::dotProduct(pos, pos) > (actor.size*actor.size)) return;
	}
	break;

	case Filter::AT_QUAD:
	{
		if (pos.x() < -actor.size || pos.x() > actor.size || pos.y() < -actor.size || pos.y() > actor.size) return;
	}
	break;

	case Filter::AT_TRIANGLE:
	{
		const float sqrt_0_75 = 0.866025f; //sqrt(0.75)
		const float s1_len = 1.93185165258f; //sqrt(2+2*sqrt(0.75)) 

		if (pos.x() < -sqrt_0_75 * actor.size) return;

		QPointF s1 = QPointF(-(1+ sqrt_0_75 )*actor.size, -0.5*actor.size); // -(1+sqrt(0.75)), -0.5
		QPointF p = QPointF(pos.x() - actor.size, pos.y());
		const float p_len = sqrt(QPointF::dotProduct(p, p));

		if (QPointF::dotProduct(p, s1) < sqrt_0_75*p_len*s1_len*actor.size) return;
	}
	break;
	
	default: return;
	}

	QString tips = QString("-------------\nID:%1\nPos:%2,%3\n%4\n")
		.arg(actor.actorID)
		.arg(actor.pos.x()).arg(actor.pos.y())
		.arg(actor.info);

	mouseTips += tips;
}

//--------------------------------------------------------------------------------------------
void Map2DChild::initCachedObject(void)
{
	for (int i = 0; i < MAX_COLOR_COUNTS; i++) {
		m_cachedPen[i] = nullptr;
		m_cachedBrush[i] = nullptr;
	}
}

//--------------------------------------------------------------------------------------------
void Map2DChild::deleteCachedObject(void)
{
	for (int i = 0; i < MAX_COLOR_COUNTS; i++) {
		if (m_cachedPen[i] != nullptr) delete m_cachedPen[i];
		if (m_cachedBrush[i] != nullptr) delete m_cachedBrush[i];
	}
}

//--------------------------------------------------------------------------------------------
QPen& Map2DChild::getCachedPen(uint16_t color)
{
	QPen* pen = m_cachedPen[color & 0xFFF];
	if (pen != nullptr) return *pen;

	pen = m_cachedPen[color & 0xFFF] = new QPen(Filter::toQColor(color));
	return *pen;
}

//--------------------------------------------------------------------------------------------
QBrush& Map2DChild::getCachedBrush(uint16_t color)
{
	QBrush* brush = m_cachedBrush[color & 0xFFF];
	if (brush != nullptr) return *brush;

	brush = m_cachedBrush[color & 0xFFF] = new QBrush(Filter::toQColor(color));
	return *brush;
}
