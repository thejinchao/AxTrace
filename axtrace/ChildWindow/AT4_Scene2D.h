/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#pragma once
#include "AT4_Interface.h"
#include <QJsonObject>

class Scene2DMessage;
class Begin2DSceneMessage;
class Update2DActorMessage;
class End2DSceneMessage;
class Add2DActorLogMessage;
struct Actor2DFilterResult;

class Scene2D
{
public:
	enum ActorType
	{
		AT_CIRCLE,
		AT_QUAD,
		AT_TRIANGLE,
	};

	struct Actor
	{
		qint64 actorID;
		QPointF pos;
		qreal dir;
		ActorType type;
		quint16 borderColor;
		quint16 fillColor;
		qreal size;
		QString info;
	};

	struct PositionSnap
	{
		QPointF pos;
		qreal dir;
		MessageTime time;
	};

	struct PositionTail
	{
		qint64 actorID;
		QQueue<PositionSnap> posTail;
	};

	struct ActorHistory
	{
		qint64 actorID;
		QQueue<QString> logHistory;
	};

public:
	void beginScene(const Begin2DSceneMessage* msg);
	void updateActor(const Update2DActorMessage* msg, const Actor2DFilterResult& filterResult);
	void endScene(const End2DSceneMessage* msg);
	void addActorLog(Add2DActorLogMessage* msg);

	void clean(void);

	typedef std::function<void(const Actor&)> ActorWalkFunc;
	void actorWalk(ActorWalkFunc walkFunc);

	const QString& getSceneName(void) const { return m_sceneName; }
	const QRectF& getSceneRect(void) const { return m_sceneRect; }
	quint32 getFrameIndex(void) const {	return m_frameIndex; }
	qint32 getActorCounts(void) const { return m_actorMap[m_actorMapIndex].count(); }

	QString getActorBriefInfo(const Actor& actor) const;
	QString getActorBriefInfo(qint64 id) const;

	QString getActorDetailInfo(const Actor& actor) const;

	void enablePositionTail(qint64 id, bool enable);
	const PositionTail* getPositionTail(qint64 id) const;

public:
	void addGridDefine(const QSizeF& gridSize, const QPointF& gridPoint);
	bool isGridDefined(void) const { return m_gridDefined; }
	const QSizeF& getGridSize(void) const { return m_gridSize; }
	const QPointF& getGridPoint(void) const { return m_gridPoint; }

	void addSquareShape(const QRectF& square);
	typedef std::function<void(const QRectF&)> SquareWalkFunc;
	void squareShapeWalk(SquareWalkFunc walkFunc);

	void addCircleShape(const QPointF& center, qreal radius);
	typedef std::function<void(const QPointF&, qreal)> CircleWalkFunc;
	void circleShapeWalk(CircleWalkFunc walkFunc);

private:
	QString			m_sceneName;
	QRectF			m_sceneRect;
	quint32			m_actorMapIndex;
	quint32			m_frameIndex;

	bool			m_gridDefined;
	QSizeF			m_gridSize;
	QPointF			m_gridPoint;

	typedef QVector< QRectF > SquareShapeList;
	SquareShapeList	m_squareShapes;

	typedef QVector<QPair<QPointF, qreal>> CircleShapeList;
	CircleShapeList m_circleShapes;

	typedef QHash< qint64, Actor > ActorMap;
	ActorMap m_actorMap[2];

	bool			m_updating;
	QRectF			m_updatingRect;
	
	typedef QHash< qint64, ActorHistory > ActorHistoryMap;
	ActorHistoryMap m_actorHistory;

	typedef QHash< qint64, PositionTail > ActorTailMap;
	ActorTailMap m_actorTail;
public:
	Scene2D(Scene2DMessage* msg);
	~Scene2D();
};
