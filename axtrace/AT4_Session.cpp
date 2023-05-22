/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#include "stdafx.h"
#include "AT4_Session.h"
#include "AT4_Message.h"
#include "AT4_System.h"
#include "AT4_Incoming.h"

//--------------------------------------------------------------------------------------------
Session::Session(int32_t id, cyclone::TcpConnectionPtr connPtr)
	: m_id(id)
	, m_bShakehand(false)
	, m_connection(connPtr)
	, m_processID(0)
	, m_threadID(0)
	, m_userData(-1)
{

}

//--------------------------------------------------------------------------------------------
Session::~Session()
{

}

//--------------------------------------------------------------------------------------------
QString Session::getPeerAddress(void) const
{
	return QString("%0:%1")
		.arg(m_connection->get_peer_addr().get_ip())
		.arg(m_connection->get_peer_addr().get_port());
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
qint32 SessionManager::getSessionCounts(void) const
{ 
	QMutexLocker locker(&m_lock);

	return m_sessionMap.size();
}

//--------------------------------------------------------------------------------------------
void SessionManager::onSessionConnected(cyclone::TcpConnectionPtr connPtr)
{
	QMutexLocker locker(&m_lock);

	SessionPtr session = SessionPtr(new Session(connPtr->get_id(), connPtr));

	m_sessionMap.insert(session->getID(), session);
}

//--------------------------------------------------------------------------------------------
void SessionManager::onSessionClose(cyclone::TcpConnectionPtr connPtr)
{
	QMutexLocker locker(&m_lock);

	m_sessionMap.remove(connPtr->get_id());
}

//--------------------------------------------------------------------------------------------
void SessionManager::walk(SessionManager::WalkCallback callback)
{
	for (auto it = m_sessionMap.begin(); it != m_sessionMap.end(); it++) {
		callback(it.key(), it.value());
	}
}
