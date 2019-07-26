/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include "AT4_Interface.h"

class Message
{
public:
	virtual void build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf) = 0;
	virtual unsigned int getType(void) const = 0;

	unsigned int getProcessID(void) const { return m_processID; }
	unsigned int getThreadID(void) const { return m_threadID; }
	const axtrace_time_s& getTime(void) const { return m_time; }

protected:
	unsigned int	m_processID;
	unsigned int	m_threadID;
	axtrace_time_s	m_time;

protected:
	static int _lua_get_type(lua_State *L);
	static int _lua_get_process_id(lua_State *L);
	static int _lua_get_thread_id(lua_State *L);

public:
	Message();
	virtual ~Message();

public:
	virtual void reccycleMessage(void) = 0;
};

typedef QVector< Message* > MessageVector;

#define DEFINE_POOL(Name) \
public: \
	static int debugCounts(void) { return (int)s_messagePool.size(); } \
	static Name* allocMessage(void) \
	{ \
		if (s_messagePool.empty()) { \
			return new Name(); \
		} \
		else { \
			Name* msg = s_messagePool.front(); \
			s_messagePool.pop_front(); \
			return msg; \
		} \
	} \
	virtual void reccycleMessage() \
	{ \
		s_messagePool.push_back(this); \
	} \
	static void deletePool(void) \
	{ \
		for(auto msg : s_messagePool) { \
			delete msg; \
		} \
		s_messagePool.clear(); \
	} \
private: \
	static QQueue<Name*> s_messagePool;

class LogMessage : public Message
{
public:
	static const char* MetaName;
	static void _luaopen(lua_State *L);

	virtual void build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf);
	virtual unsigned int getType(void) const { return AXTRACE_CMD_TYPE_LOG; }

	unsigned int getLogType(void) const { return m_logType; }
	const QString& getLog(void) const { return m_log; }

private:
	unsigned int m_logType;
	QString m_log;

protected:
	static int _lua_get_log_type(lua_State *L);
	static int _lua_get_log(lua_State *L);

public:
	LogMessage();
	virtual ~LogMessage();

	DEFINE_POOL(LogMessage);
};

class ValueMessage : public Message 
{
public:
	static const char* MetaName;
	static void _luaopen(lua_State *L);

	virtual void build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf);
	virtual unsigned int getType(void) const { return AXTRACE_CMD_TYPE_VALUE; }

	const QString& getName(void) const { return m_name; }
	void getValueAsString(QString& value) const;

private:
	QString m_name;

	unsigned int	m_valueType;
	size_t			m_valueSize;
	void*			m_valueBuf;

	enum { STANDARD_VALUE_SIZE = 32 };
	unsigned char m_standValueBuf[STANDARD_VALUE_SIZE];

protected:
	static int _lua_get_value(lua_State *L);

public:
	ValueMessage();
	virtual ~ValueMessage();

	DEFINE_POOL(ValueMessage);
};

class Begin2DSceneMessage : public Message
{
public:
	static const char* MetaName;
	static void _luaopen(lua_State *L);

	virtual void build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf);
	virtual unsigned int getType(void) const { return AXTRACE_CMD_TYPE_2D_BEGIN_SCENE; }

	const QString& getSceneName(void) const { return m_sceneName; }
	const QRectF& getSceneRect(void) const { return m_sceneRect; }
	const QJsonObject& getSceneDefine(void) const { return m_sceneDefine; }

private:
	QString			m_sceneName;
	QRectF			m_sceneRect;
	QJsonObject		m_sceneDefine;

public:
	Begin2DSceneMessage();
	virtual ~Begin2DSceneMessage();

	DEFINE_POOL(Begin2DSceneMessage);
};

class Update2DActorMessage : public Message
{
public:
	static const char* MetaName;
	static void _luaopen(lua_State *L);

	virtual void build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf);
	virtual unsigned int getType(void) const { return AXTRACE_CMD_TYPE_2D_ACTOR; }

	const QString& getSceneName(void) const { return m_sceneName; }
	qint64 getActorID(void) const { return m_actorID; }
	const QPointF& getActorPosition(void) const { return m_position; }
	qreal getActorDir(void) const { return m_dir; }
	quint32 getActorStyle(void) const { return m_style; }
	const QString& getActorInfo(void) const { return m_actorInfo; }

private:
	QString			m_sceneName;
	qint64			m_actorID;
	QPointF			m_position;
	qreal			m_dir;
	quint32			m_style;
	QString			m_actorInfo;

protected:
	static int _lua_get_actor_id(lua_State *L);
	static int _lua_get_actor_position(lua_State *L);
	static int _lua_get_actor_dir(lua_State *L);
	static int _lua_get_actor_style(lua_State *L);
	static int _lua_get_actor_info(lua_State *L);

public:
	Update2DActorMessage();
	virtual ~Update2DActorMessage();

	DEFINE_POOL(Update2DActorMessage);
};


class End2DSceneMessage : public Message
{
public:
	static const char* MetaName;
	static void _luaopen(lua_State *L);

	virtual void build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf);
	virtual unsigned int getType(void) const { return AXTRACE_CMD_TYPE_2D_END_SCENE; }

	const QString& getSceneName(void) const { return m_sceneName; }

private:
	QString			m_sceneName;

protected:
	static int _lua_get_actor_id(lua_State *L);

public:
	End2DSceneMessage();
	virtual ~End2DSceneMessage();

	DEFINE_POOL(End2DSceneMessage);
};

