/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#pragma once

#include <cy_network.h>
#include <QMutex>
#include "AT4_Interface.h"

class ShakehandMessage;

class Session
{
public:
	int32_t getID(void) const { return m_id; }

	const QString& getSessionName(void) const { return m_sessionName; }
	uint32_t getProcessID(void) const { return m_processID; }
	uint32_t getThreadID(void) const { return m_threadID; }
	QString getPeerAddress(void) const;
	bool isHandshaked(void) const { return m_bShakehand; }

	bool onSessionShakehand(const ShakehandMessage* message);
	void closeConnection(void);

	void setUserData(int32_t userData) { m_userData = userData; }
	int32_t getUserData(void) const { return m_userData; }
private:
	int32_t						m_id;
	bool						m_bShakehand;
	cyclone::TcpConnectionPtr	m_connection;
	uint32_t					m_processID;
	uint32_t 					m_threadID;
	QString						m_sessionName;
	int32_t						m_userData;

public:
	Session(int32_t id, cyclone::TcpConnectionPtr connPtr);
	virtual ~Session();

	friend class SessionManager;
};

typedef QSharedPointer<Session> SessionPtr;

class SessionManager
{
public:
	//Thread safe functions
	SessionPtr findSession(int32_t sessionID);
	qint32 getSessionCounts(void) const;

	void onSessionConnected(cyclone::TcpConnectionPtr connPtr);
	void onSessionClose(cyclone::TcpConnectionPtr connPtr);

	typedef std::function<void(int32_t id, SessionPtr session)> WalkCallback;
	void walk(WalkCallback callback);
private:
	typedef QMap<int32_t, SessionPtr> SessionMap;

	mutable QMutex m_lock;
	SessionMap	m_sessionMap;

public:
	SessionManager();
	virtual ~SessionManager();
};
