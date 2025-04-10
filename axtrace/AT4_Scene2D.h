﻿/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#pragma once

#include <QJsonObject>

class Begin2DSceneMessage;
class Update2DActorMessage;
class End2DSceneMessage;
class Add2DActorLogMessage;

#include "AT4_Filter.h"
#include "AT4_Interface.h"

class Scene2D
{
public:
	struct Actor
	{
		qint64 actorID;
		QPointF pos;
		qreal dir;
		Filter::Actor2DType type;
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
	void updateActor(const Update2DActorMessage* msg, const Filter::Actor2DResult& filterResult);
	void endScene(const End2DSceneMessage* msg);
	void addActorLog(Add2DActorLogMessage* msg);

	void clean(void);

	typedef std::function<void(const Actor&)> ActorWalkFunc;
	void walk(ActorWalkFunc walkFunc);

	const QString& getSceneName(void) const { return m_sceneName; }
	const QRectF& getSceneRect(void) const { return m_sceneRect; }
	quint32 getFrameIndex(void) const {	return m_frameIndex; }
	qint32 getActorCounts(void) const { return m_actorMap[m_actorMapIndex].count(); }

	bool isGridDefined(void) const { return m_gridDefined; }
	const QSizeF& getGridSize(void) const { return m_gridSize; }
	const QPointF& getGridPoint(void) const { return m_gridPoint; }

	QString getActorBriefInfo(const Actor& actor) const;
	QString getActorBriefInfo(qint64 id) const;

	QString getActorDetailInfo(const Actor& actor) const;

	void enablePositionTail(qint64 id, bool enable);
	const PositionTail* getPositionTail(qint64 id) const;

private:
	void _parserSceneDefine(const QJsonObject& sceneInfo);

private:
	QString			m_sceneName;
	QRectF			m_sceneRect;
	quint32			m_actorMapIndex;
	quint32			m_frameIndex;

	bool			m_gridDefined;
	QSizeF			m_gridSize;
	QPointF			m_gridPoint;

	typedef QHash< qint64, Actor > ActorMap;
	ActorMap m_actorMap[2];

	bool			m_updating;
	QRectF			m_updatingRect;
	QJsonObject		m_updatingSceneDefine;
	
	typedef QHash< qint64, ActorHistory > ActorHistoryMap;
	ActorHistoryMap m_actorHistory;

	typedef QHash< qint64, PositionTail > ActorTailMap;
	ActorTailMap m_actorTail;
public:
	Scene2D(Begin2DSceneMessage* msg);
	~Scene2D();
};
