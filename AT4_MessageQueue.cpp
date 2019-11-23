/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_MessageQueue.h"
#include "AT4_Interface.h"
#include "AT4_System.h"
#include "AT4_MainWindow.h"
#include "AT4_Incoming.h"

//--------------------------------------------------------------------------------------------
MessageQueue::MessageQueue()
{
	m_ring_buf = new cyclone::RingBuf();
	m_counts = 0;
}

//--------------------------------------------------------------------------------------------
MessageQueue::~MessageQueue()
{
	delete m_ring_buf; m_ring_buf = 0;
}

//--------------------------------------------------------------------------------------------
void MessageQueue::insertMessage(cyclone::RingBuf* buf, size_t msg_length, const QTime& timeNow, qint32 sessionID)
{
	MessageTime t;
	t.hour = timeNow.hour();
	t.minute = timeNow.minute();
	t.second = timeNow.second();
	t.milliseconds = timeNow.msec();

	{
		QMutexLocker locker(&m_lock);

		m_ring_buf->memcpy_into(&sessionID, sizeof(qint32));
		m_ring_buf->memcpy_into(&t, sizeof(MessageTime));
		buf->copyto(m_ring_buf, msg_length);
		
		m_counts++;

		QCoreApplication::postEvent(System::getSingleton()->getMainWindow(), new MainWindow::AxTraceEvent());
	}
}

//--------------------------------------------------------------------------------------------
Message* MessageQueue::_popMessage(void)
{
	MessageTime traceTime;

	//pop session id
	qint32 sessionID;
	size_t len = m_ring_buf->memcpy_out(&sessionID, sizeof(qint32));
	Q_ASSERT(len == sizeof(qint32));

	//pop time and session id
	len = m_ring_buf->memcpy_out(&traceTime, sizeof(MessageTime));
	Q_ASSERT(len == sizeof(MessageTime));

	axtrace_head_s head;
	len = m_ring_buf->peek(0, &head, sizeof(head));
	Q_ASSERT(len == sizeof(axtrace_head_s));

	//find session
	SessionPtr session = System::getSingleton()->getSessionManager()->findSession(sessionID);
	if (session == nullptr)
	{
		//discard message
		m_ring_buf->discard(head.length);
		return nullptr;
	}

	Message* message = nullptr;
	switch (head.type)
	{
	case AXTRACE_CMD_TYPE_SHAKEHAND:
	{
		message = ShakehandMessage::allocMessage(session, traceTime);
	}
	break;

	case AXTRACE_CMD_TYPE_LOG:
	{
		message = LogMessage::allocMessage(session, traceTime);
	}
	break;

	case AXTRACE_CMD_TYPE_VALUE:
	{
		message = ValueMessage::allocMessage(session, traceTime);
	}
	break;

	case AXTRACE_CMD_TYPE_2D_BEGIN_SCENE:
	{
		message = Begin2DSceneMessage::allocMessage(session, traceTime);
	}
	break;

	case AXTRACE_CMD_TYPE_2D_ACTOR:
	{
		message = Update2DActorMessage::allocMessage(session, traceTime);
	}
	break;

	case AXTRACE_CMD_TYPE_2D_END_SCENE:
	{
		message = End2DSceneMessage::allocMessage(session, traceTime);
	}
	break;

	case AXTRACE_CMD_TYPE_2D_ACTOR_LOG:
	{
		message = Add2DActorLogMessage::allocMessage(session, traceTime);
	}
	break;

	default:
		return message;
	}

	if (!(message->build(head, m_ring_buf)))
	{
		//discard message
		m_ring_buf->discard(head.length);

		//Close Net Connection 
		session->closeConnection();

		return nullptr;
	}
	return message;
}

//--------------------------------------------------------------------------------------------
void MessageQueue::popMessage(MessageVector& msgVector)
{
	if (m_counts <= 0) return;

	{
		QMutexLocker locker(&m_lock);
		msgVector.reserve(m_counts);

		do 
		{
			if (m_ring_buf->empty()) break;

			Message* msg = _popMessage();
			if (msg == nullptr) continue;

			msgVector.push_back(msg);

		} while (true);

		m_counts = 0;
	}
}
