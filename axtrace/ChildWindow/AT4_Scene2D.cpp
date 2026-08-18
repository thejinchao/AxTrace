/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#include "stdafx.h"
#include "AT4_Scene2D.h"
#include "AT4_Message.h"
#include "AT4_System.h"
#include "AT4_Config.h"
#include "AT4_LuaVirtualMachine.h"

//--------------------------------------------------------------------------------------------
Scene2D::Scene2D(Scene2DMessage* msg)
	: m_sceneName(msg->getSceneName())
	, m_actorMapIndex(0)
	, m_frameIndex(0)
	, m_updating(false)
	, m_gridDefined(false)
{

}

//--------------------------------------------------------------------------------------------
Scene2D::~Scene2D()
{

}

//--------------------------------------------------------------------------------------------
void Scene2D::beginScene(const Begin2DSceneMessage* msg)
{
	ActorMap& updatingActorsMap = m_actorMap[1-m_actorMapIndex];

	m_updating = true;
	m_updatingRect = msg->getSceneRect();
	updatingActorsMap.clear();
	//m_updatingSceneDefine = msg->getSceneDefine();
}

//--------------------------------------------------------------------------------------------
void Scene2D::updateActor(const Update2DActorMessage* msg, const Actor2DFilterResult& filterResult)
{
	if (!m_updating) return;
	ActorMap& updatingActorsMap = m_actorMap[1 - m_actorMapIndex];

	ActorMap::iterator it = updatingActorsMap.find(msg->getActorID());
	if (it == updatingActorsMap.end()) {
		Actor actor;
		actor.actorID = msg->getActorID();
		actor.pos = msg->getActorPosition();
		actor.dir = msg->getActorDir();
		actor.type = filterResult.type;
		actor.size = filterResult.size;
		actor.fillColor = filterResult.fillColor;
		actor.borderColor = filterResult.borderColor;
		actor.info = msg->getActorInfo();

		updatingActorsMap.insert(msg->getActorID(), actor);
	}
	else
	{
		Actor& actor = *it;

		actor.pos = msg->getActorPosition();
		actor.dir = msg->getActorDir();
		actor.type = filterResult.type;
		actor.size = filterResult.size;
		actor.fillColor = filterResult.fillColor;
		actor.borderColor = filterResult.borderColor;
		actor.info = msg->getActorInfo();
	}

	//add to position history
	ActorTailMap::iterator it_tail = m_actorTail.find(msg->getActorID());
	if (it_tail != m_actorTail.end()) {

		QQueue<PositionSnap>& posTail = it_tail.value().posTail;

		//is same position?
		if (!posTail.empty()) {
			const PositionSnap& lastPosition = posTail.front();
			const QPointF& newPos = msg->getActorPosition();

			if (lastPosition.pos == newPos) return;
		}

		PositionSnap posSnap;
		posSnap.pos = msg->getActorPosition();
		posSnap.dir = msg->getActorDir();
		posSnap.time = msg->getTime();

		posTail.push_back(posSnap);

		qint32 maxTailCounts = System::getSingleton()->getConfig()->getMaxActorTailCounts();
		while (posTail.size() > maxTailCounts) {
			posTail.pop_front();
		}
	}
}

//--------------------------------------------------------------------------------------------
void Scene2D::endScene(const End2DSceneMessage* msg)
{
	if (!m_updating) return;

	m_updating = false;
	m_actorMapIndex = 1 - m_actorMapIndex;
	m_frameIndex++;
	m_sceneRect = m_updatingRect;
}

//--------------------------------------------------------------------------------------------
QString Scene2D::getActorBriefInfo(qint64 id) const
{
	const ActorMap& currentMap = m_actorMap[m_actorMapIndex];

	ActorMap::const_iterator it = currentMap.find(id);
	if (it == currentMap.end()) return QString();

	return getActorBriefInfo(it.value());
}

//--------------------------------------------------------------------------------------------
void Scene2D::addActorLog(Add2DActorLogMessage* msg)
{
	ActorHistoryMap::iterator it = m_actorHistory.find(msg->getActorID());
	if (it == m_actorHistory.end())
	{
		ActorHistory history;
		history.actorID = msg->getActorID();
		history.logHistory.push_back(msg->getActorLog());

		m_actorHistory.insert(msg->getActorID(), history);
	}
	else
	{
		ActorHistory& history = *it;
		history.logHistory.push_back(msg->getActorLog());

		qint32 maxActorLogCounts = System::getSingleton()->getConfig()->getMaxActorLogCounts();
		while(history.logHistory.size()>maxActorLogCounts)
			history.logHistory.pop_front();
	}
}

//--------------------------------------------------------------------------------------------
void Scene2D::clean(void)
{
	ActorMap& actorsMap = m_actorMap[m_actorMapIndex];

	actorsMap.clear();
}

//--------------------------------------------------------------------------------------------
void Scene2D::actorWalk(Scene2D::ActorWalkFunc walkFunc)
{
	ActorMap& actorsMap = m_actorMap[m_actorMapIndex];

	for (ActorMap::iterator it = actorsMap.begin(); it != actorsMap.end(); ++it)
	{
		const Actor& actor = it.value();
		walkFunc(actor);
	}
}

//--------------------------------------------------------------------------------------------
void Scene2D::addGridDefine(const QSizeF& gridSize, const QPointF& gridPoint)
{
	m_gridSize = QSizeF(abs(gridSize.width()), abs(gridSize.height()));

	//default gridPoint
	m_gridPoint = QPointF(0, 0);

	double gridPointX = gridPoint.x();
	double gridPointY = gridPoint.y();

	double absPointX = abs(gridPointX);
	double absPointY = abs(gridPointY);

	int flagX = gridPoint.x() > 0 ? 1 : -1;
	int flagY = gridPoint.y() > 0 ? 1 : -1;
	
	if (absPointX > m_gridSize.width()) {
		gridPointX = flagX*(absPointX - (int)(absPointX / m_gridSize.width()) * m_gridSize.width());
	}
	if (gridPointY > m_gridSize.height()) {
		gridPointY = flagY * (absPointY - (int)(absPointY / m_gridSize.height()) * m_gridSize.height());
	}

	m_gridPoint = QPointF(gridPointX, gridPointY);

	m_gridDefined = true;
}

//--------------------------------------------------------------------------------------------
void Scene2D::addSquareShape(const QRectF& square)
{
	m_squareShapes.append(square);
}

//--------------------------------------------------------------------------------------------
void Scene2D::squareShapeWalk(Scene2D::SquareWalkFunc walkFunc)
{
	for (SquareShapeList::iterator it = m_squareShapes.begin(); it != m_squareShapes.end(); ++it)
	{
		const QRectF& actor = *it;
		walkFunc(actor);
	}
}

//--------------------------------------------------------------------------------------------
void Scene2D::addCircleShape(const QPointF& center, qreal radius)
{
	m_circleShapes.append(qMakePair(center, radius));
}

//--------------------------------------------------------------------------------------------
void Scene2D::circleShapeWalk(CircleWalkFunc walkFunc)
{
	for (CircleShapeList::iterator it = m_circleShapes.begin(); it != m_circleShapes.end(); ++it)
	{
		walkFunc(it->first, it->second);
	}
}

//--------------------------------------------------------------------------------------------
QString Scene2D::getActorBriefInfo(const Actor& actor) const
{
	QString brief = QString("ID:%1\nPos:%2,%3\nDir:%4")
		.arg(actor.actorID)
		.arg(actor.pos.x()).arg(actor.pos.y())
		.arg(actor.dir);

	if (!actor.info.isEmpty())
	{
		brief += "\nInfo:";
		brief += actor.info;
	}
	
	return brief;
}

//--------------------------------------------------------------------------------------------
QString Scene2D::getActorDetailInfo(const Actor& actor) const
{
	QString detailInfo = getActorBriefInfo(actor);

	//add actor log
	ActorHistoryMap::const_iterator it = m_actorHistory.find(actor.actorID);
	if (it == m_actorHistory.end()) return detailInfo;

	//get log info
	const ActorHistory& history = it.value();
	if (!history.logHistory.empty())
	{
		detailInfo += "\nLog:";
		foreach(const QString& log, history.logHistory)
		{
			detailInfo += "\n";
			detailInfo += log;
		}
	}
	return detailInfo;
}

//--------------------------------------------------------------------------------------------
void Scene2D::enablePositionTail(qint64 id, bool enable)
{
	ActorTailMap::iterator it = m_actorTail.find(id);

	if (enable && it == m_actorTail.end()) {
		PositionTail posTail;
		posTail.actorID = id;
		m_actorTail.insert(id, posTail);
	}

	if (!enable && it != m_actorTail.end()) {
		m_actorTail.remove(id);
	}
}

//--------------------------------------------------------------------------------------------
const Scene2D::PositionTail* Scene2D::getPositionTail(qint64 id) const
{
	ActorTailMap::const_iterator it = m_actorTail.find(id);

	if (it == m_actorTail.end()) {
		return nullptr;
	}
	return &(it.value());
}
