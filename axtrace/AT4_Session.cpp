/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_Session.h"
#include "AT4_Message.h"
#include "AT4_System.h"
#include "AT4_Incoming.h"

//--------------------------------------------------------------------------------------------
Session::Session(cyclone::ConnectionPtr connPtr)
	: m_bShakehand(false)
	, m_connection(connPtr)
	, m_processID(0)
	, m_threadID(0)
{

}

//--------------------------------------------------------------------------------------------
Session::~Session()
{

}

//--------------------------------------------------------------------------------------------
bool Session::onSessionShakehand(const ShakehandMessage* message)
{
	if (m_bShakehand) return false;

	m_processID = message->getProcessID();
	m_threadID = message->getThreadID();
	m_sessionName = message->geetSessionName();
	m_bShakehand = true;
	return true;
}

//--------------------------------------------------------------------------------------------
void Session::closeConnection(void)
{
	System::getSingleton()->getIncoming()->kickConnection(m_connection);
}

//--------------------------------------------------------------------------------------------
SessionManager::SessionManager()
{

}

//--------------------------------------------------------------------------------------------
SessionManager::~SessionManager()
{

}

//--------------------------------------------------------------------------------------------
SessionPtr SessionManager::findSession(int32_t sessionID)
{
	QMutexLocker locker(&m_lock);

	//find connected session
	SessionMap::const_iterator it = m_sessionMap.find(sessionID);
	if (it == m_sessionMap.end()) return SessionPtr(nullptr);
	
	return it.value();
}

//--------------------------------------------------------------------------------------------
void SessionManager::onSessionConnected(cyclone::ConnectionPtr connPtr)
{
	QMutexLocker locker(&m_lock);

	SessionPtr session = SessionPtr(new Session(connPtr));

	m_sessionMap.insert(connPtr->get_id(), session);
}

//--------------------------------------------------------------------------------------------
void SessionManager::onSessionClose(cyclone::ConnectionPtr connPtr)
{
	QMutexLocker locker(&m_lock);

	m_sessionMap.remove(connPtr->get_id());
}