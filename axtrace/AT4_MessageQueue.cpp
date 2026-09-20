/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#include "stdafx.h"
#include "AT4_MessageQueue.h"
#include "AT4_Interface.h"
#include "AT4_System.h"
#include "AT4_MainWindow.h"
#include "AT4_Incoming.h"

//--------------------------------------------------------------------------------------------
MessageQueue::MessageQueue()
	: m_ringBuf(DEFAULT_RINGBUF_SIZE)
	, m_msgCountsInBuff(0)
{
}

//--------------------------------------------------------------------------------------------
MessageQueue::~MessageQueue()
{
}

//--------------------------------------------------------------------------------------------
void MessageQueue::insertMessage(cyclone::RingBuf& sourceBuf, size_t msgLength, const QDateTime& timeNow, qint32 sessionID)
{
	if (sourceBuf.size() < msgLength) return;

	MessageTime t;
	t.epochTime = timeNow.toMSecsSinceEpoch();

	{
		QMutexLocker locker(&m_lock);

		m_ringBuf.memcpy_into(&sessionID, sizeof(qint32));
		m_ringBuf.memcpy_into(&t, sizeof(MessageTime));
		sourceBuf.moveto(m_ringBuf, msgLength);
		
		m_msgCountsInBuff++;

		QCoreApplication::postEvent(System::getSingleton()->getMainWindow(), new MainWindow::AxTraceEvent(MainWindow::AxTraceEvent::ET_Message));
	}
}

//--------------------------------------------------------------------------------------------
Message* MessageQueue::_popMessage(void)
{
	MessageTime traceTime;

	qint32 sessionID = 0;
	axtrace_head_s head;
	SessionPtr session = nullptr;

	{
		QMutexLocker locker(&m_lock);

		//pop session id
		size_t len = m_ringBuf.memcpy_out(&sessionID, sizeof(qint32));
		Q_ASSERT(len == sizeof(qint32));

		//pop time and session id
		len = m_ringBuf.memcpy_out(&traceTime, sizeof(MessageTime));
		Q_ASSERT(len == sizeof(MessageTime));

		len = m_ringBuf.peek(0, &head, sizeof(head));
		Q_ASSERT(len == sizeof(axtrace_head_s));

		//find session
		session = System::getSingleton()->getSessionManager()->findSession(sessionID);
		if (session == nullptr)
		{
			//discard message if session closed
			m_msgCountsInBuff--;
			m_ringBuf.discard(head.length);
			return nullptr;
		}

		//read message to temp buffer
		if (m_tempBuf.size() < head.length)
		{
			m_tempBuf.resize(head.length);
		}
		m_ringBuf.memcpy_out(m_tempBuf.data(), head.length);

		m_msgCountsInBuff--;
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
		return nullptr;
	}
	
	if (!(message->build(QByteArrayView(m_tempBuf.data(), head.length))))
	{
		//Close Net Connection 
		session->closeConnection();
		return nullptr;
	}
	return message;
}

//--------------------------------------------------------------------------------------------
void MessageQueue::popMessage(MessageVector& msgVector)
{
	qint32 msgCountsOfThisTick = m_msgCountsInBuff;
	if (msgCountsOfThisTick <= 0) return;
	msgVector.reserve(msgVector.size() + msgCountsOfThisTick);

	do 
	{
		Message* msg = _popMessage();
		if (msg)
		{
			msgVector.push_back(msg);
		}
	} while (m_msgCountsInBuff>0 && --msgCountsOfThisTick > 0);
}
