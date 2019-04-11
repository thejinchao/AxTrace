#include "stdafx.h"
#include "AT4_MessageQueue.h"
#include "AT4_Interface.h"
#include "AT4_System.h"
#include "AT4_MainWindow.h"

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
void MessageQueue::insertMessage(cyclone::RingBuf* buf, size_t msg_length, const QTime& timeNow)
{
	axtrace_time_s t;
	t.hour = timeNow.hour();
	t.minute = timeNow.minute();
	t.second = timeNow.second();
	t.milliseconds = timeNow.msec();

	{
		QMutexLocker locker(&m_lock);

		m_ring_buf->memcpy_into(&t, sizeof(axtrace_time_s));
		buf->copyto(m_ring_buf, msg_length);
		
		m_counts++;

		QCoreApplication::postEvent(System::getSingleton()->getMainWindow(), new MainWindow::AxTraceEvent());
	}
}

//--------------------------------------------------------------------------------------------
Message* MessageQueue::_popMessage(void)
{
	axtrace_time_s traceTime;

	size_t len = m_ring_buf->memcpy_out(&traceTime, sizeof(axtrace_time_s));
	assert(len == sizeof(axtrace_time_s));

	axtrace_head_s head;
	len = m_ring_buf->peek(0, &head, sizeof(head));
	assert(len == sizeof(axtrace_head_s));

	Message* message = nullptr;
	switch (head.type)
	{
	case AXTRACE_CMD_TYPE_LOG:
	{
		message = LogMessage::allocMessage();
	}
	break;

	case AXTRACE_CMD_TYPE_VALUE:
	{
		message = ValueMessage::allocMessage();
	}
	break;

	case AXTRACE_CMD_TYPE_2D_BEGIN_SCENE:
	{
		message = Begin2DSceneMessage::allocMessage();
	}
	break;

	case AXTRACE_CMD_TYPE_2D_ACTOR:
	{
		message = Update2DActorMessage::allocMessage();
	}
	break;

	case AXTRACE_CMD_TYPE_2D_END_SCENE:
	{
		message = End2DSceneMessage::allocMessage();
	}
	break;

	default:
		return message;
	}

	message->build(traceTime, head, m_ring_buf);
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
			assert(msg != 0);
			if (msg == nullptr) continue;

			msgVector.push_back(msg);

		} while (true);

		m_counts = 0;
	}
}
