/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include <QJsonObject>

class Begin2DSceneMessage;
class Update2DActorMessage;
class End2DSceneMessage;

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

		QString buildBriefInfo(void) const;
		QString buildDetailInfo(void) const;
	};

public:
	void beginScene(const Begin2DSceneMessage* msg);
	void updateActor(const Update2DActorMessage* msg, const Filter::Actor2DResult& filterResult);
	void endScene(const End2DSceneMessage* msg);

	void clean(void);

	typedef std::function<void(const Actor&)> ActorWalkFunc;
	void walk(ActorWalkFunc walkFunc);

	const QString& getSceneName(void) const { return m_sceneName; }
	const QRectF& getSceneRect(void) const { return m_sceneRect; }

	bool isGridDefined(void) const { return m_gridDefined; }
	const QSizeF& getGridSize(void) const { return m_gridSize; }
	const QPointF& getGridPoint(void) const { return m_gridPoint; }

private:
	void _parserSceneDefine(const QJsonObject& sceneInfo);

private:
	QString			m_sceneName;
	QRectF			m_sceneRect;
	int				m_actorMapIndex;

	bool			m_gridDefined;
	QSizeF			m_gridSize;
	QPointF			m_gridPoint;

	typedef QHash< qint64, Actor > ActorMap;
	ActorMap m_actorMap[2];

	bool			m_updating;
	QRectF			m_updatingRect;
	QJsonObject		m_updatingSceneDefine;
	
public:
	Scene2D(Begin2DSceneMessage* msg);
	~Scene2D();
};
