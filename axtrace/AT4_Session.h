﻿/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include <cy_network.h>
#include <QMutex>
#include "AT4_Interface.h"

class ShakehandMessage;

class Session
{
public:
	const QString& getSessionName(void) const { return m_sessionName; }
	uint32_t getProcessID(void) const { return m_processID; }
	uint32_t getThreadID(void) const { return m_threadID; }
	bool isHandshaked(void) const { return m_bShakehand; }

	bool onSessionShakehand(const ShakehandMessage* message);
	void closeConnection(void);

private:
	bool						m_bShakehand;
	cyclone::TcpConnectionPtr	m_connection;
	uint32_t					m_processID;
	uint32_t 					m_threadID;
	QString						m_sessionName;

public:
	Session(cyclone::TcpConnectionPtr connPtr);
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

private:
	typedef QMap<int32_t, SessionPtr> SessionMap;

	mutable QMutex m_lock;
	SessionMap	m_sessionMap;

public:
	SessionManager();
	virtual ~SessionManager();
};
