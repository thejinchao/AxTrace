/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_MessageQueue.h"
#include "AT_Util.h"
#include "AT_System.h"
#include "AT_MainFrame.h"

#include "AT_Interface.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
MessageQueue::MessageQueue()
	: m_hNotEmptySignal(0)
{
	m_ring_buf = new cyclone::RingBuf();

	InitializeCriticalSection(&m_criticalSection);
	m_hNotEmptySignal = CreateEvent(0, TRUE, FALSE, 0);
}

//--------------------------------------------------------------------------------------------
MessageQueue::~MessageQueue()
{
	delete m_ring_buf; m_ring_buf = 0;
	DeleteCriticalSection(&m_criticalSection);
	CloseHandle(m_hNotEmptySignal);
}

//--------------------------------------------------------------------------------------------
bool MessageQueue::insertMessage(cyclone::RingBuf* buf, size_t msg_length, const LPSYSTEMTIME tTime)
{
	AXIATRACE_TIME traceTime;
	traceTime.wHour = tTime->wHour;
	traceTime.wMinute = tTime->wMinute;
	traceTime.wSecond = tTime->wSecond;
	traceTime.wMilliseconds = tTime->wMilliseconds;

	//enter lock
	{
		AutoLock autoLock(&m_criticalSection);

		//copy time fist
		m_ring_buf->memcpy_into(&traceTime, sizeof(traceTime));
		//copy trace memory
		buf->copyto(m_ring_buf, msg_length);

		//Set singnal
		SetEvent(m_hNotEmptySignal);
		PostMessage(System::getSingleton()->getMainFrame()->m_hWnd, MainFrame::WM_ON_AXTRACE_MESSAGE, 0, 0);

	}
	return true;
}

//--------------------------------------------------------------------------------------------
Message* MessageQueue::_popMessage(void)
{
	void* rc = 0;
	
	AXIATRACE_TIME traceTime;
	size_t len = m_ring_buf->memcpy_out(&traceTime, sizeof(traceTime));
	//rc = ringbuf_memcpy_from(&traceTime, m_ringBuf, sizeof(traceTime));
	assert(len == sizeof(traceTime));

	axtrace_head_s head;
	len = m_ring_buf->peek(0, &head, sizeof(head));
	//rc = ringbuf_memcpy_from(&head, m_ringBuf, sizeof(head));
	assert(len==sizeof(head));

	Message* message = 0;
	switch(head.type)
	{
	case AXTRACE_CMD_TYPE_LOG:
		{
			LogMessage* msg = new LogMessage();
			msg->build(traceTime, head, m_ring_buf);
			message = msg;
		}
		break;
	case AXTRACE_CMD_TYPE_VALUE:
		{
			ValueMessage* msg = new ValueMessage();
			msg->build(traceTime, head, m_ring_buf);
			message = msg;
		}
		break;
	default: assert(false); break;
	}

	return message;
}

//--------------------------------------------------------------------------------------------
void MessageQueue::processMessage(MessageVector& msgVector)
{
	//is there any message?
	if(WAIT_OBJECT_0 != WaitForSingleObject(m_hNotEmptySignal, 0)) return;

	//enter lock
	{
		AutoLock autoLock(&m_criticalSection);
		
		do
		{
			if (m_ring_buf->empty()) break;

			Message* msg = _popMessage();
			assert(msg != 0);

			msgVector.push_back(msg);
		}while(true);

		ResetEvent(m_hNotEmptySignal);
	}
	//OutputDebugString(L"-----------[3]----------- Receive by main thread!");
}

}
