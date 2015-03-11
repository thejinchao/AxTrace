/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_MessageQueue.h"
#include "AT_Util.h"
#include "AT_System.h"
#include "AT_MainFrame.h"

#include <ATD_Interface.h>
#include "AT_Interface.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
MessageQueue::MessageQueue()
	: m_hNotEmptySignal(0)
{
	m_ringBuf = ringbuf_new(DEFAULT_RINGBUF_SIZE);
	m_ring_buf = new cyclone::RingBuf();

	InitializeCriticalSection(&m_criticalSection);
	m_hNotEmptySignal = CreateEvent(0, TRUE, FALSE, 0);
}

//--------------------------------------------------------------------------------------------
MessageQueue::~MessageQueue()
{
	delete m_ring_buf; m_ring_buf = 0;
	ringbuf_free(&m_ringBuf);
	DeleteCriticalSection(&m_criticalSection);
	CloseHandle(m_hNotEmptySignal);
}

//--------------------------------------------------------------------------------------------
bool MessageQueue::_checkMessageValid(const void* pMessage, size_t size)
{
	//assert(size>sizeof(AXIATRACE_DATAHEAD));
	if(size<=sizeof(AXIATRACE_DATAHEAD)) return false;

	AXIATRACE_DATAHEAD head;
	memcpy(&head, pMessage, sizeof(head));

	//assert(head.dwContentLen == size-sizeof(head));
	if(head.wContentLen != size-sizeof(head)) return false;

	//TODO: more check for special message valid

	return true;
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
bool MessageQueue::insertMessage(const char* pMessage, size_t size, const LPSYSTEMTIME tTime)
{
	bool isValid = _checkMessageValid(pMessage, size);
	if(!isValid) return false;

	//need size insert into ringbuf
	size_t needSize = size + sizeof(AXIATRACE_TIME);

	//enter lock
	{
		AutoLock autoLock(&m_criticalSection);

		//try realloc ringbuf
		do
		{
			//can insert now?
			if(needSize < ringbuf_bytes_free(m_ringBuf)) break;

			//alloc new size
			size_t size_now = ringbuf_capacity(m_ringBuf);
			size_t size_least = ringbuf_bytes_used(m_ringBuf) + needSize;
			size_t size_new = size_now*2;
			while(size_new<size_least && size_new<MAX_RINGBUF_SIZE) size_new *= 2;

			//overflow?
			if(size_new>=size_least)
			{
				ringbuf_t newRingBuf = ringbuf_new(size_new);
				ringbuf_copy(newRingBuf, m_ringBuf, ringbuf_bytes_used(m_ringBuf));
				ringbuf_free(&m_ringBuf);
				m_ringBuf = newRingBuf;
			}
			else
			{
				//pop oldest message
				if(!_skipMessage()) return false;
			}
		}while(true);

		AXIATRACE_TIME traceTime;
		traceTime.wHour = tTime->wHour;
		traceTime.wMinute = tTime->wMinute;
		traceTime.wSecond = tTime->wSecond;
		traceTime.wMilliseconds = tTime->wMilliseconds;

		//insert message
		ringbuf_memcpy_into(m_ringBuf, &traceTime, sizeof(traceTime));
		ringbuf_memcpy_into(m_ringBuf, pMessage, size);

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
	case AXTRACE_CMD_TYPE_TRACE:
		{
			LogMessage* msg = new LogMessage();
			msg->build(traceTime, head, m_ring_buf);
			message = msg;
		}
		break;
	case AXTRACE_CMD_TYPE_VALUE:
		{
#if 0
			ValueMessage* msg = new ValueMessage();
			msg->build(traceTime, head, m_ring_buf);
			message = msg;
#endif
		}
		break;
	default: assert(false); break;
	}

	return message;
}

//--------------------------------------------------------------------------------------------
bool MessageQueue::_skipMessage(void)
{
	size_t usedSize = ringbuf_bytes_used(m_ringBuf);
	if(usedSize<sizeof(AXIATRACE_DATAHEAD)+sizeof(AXIATRACE_TIME)) return false;

	AXIATRACE_DATAHEAD head;
	void* rc = ringbuf_memcpy_from(&head, m_ringBuf, sizeof(head));
	assert(rc!=0);

	ringbuf_memcpy_from(0, m_ringBuf, sizeof(AXIATRACE_TIME));
	ringbuf_memcpy_from(0, m_ringBuf, sizeof(head.wContentLen));

	return true;
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
