/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once

#include "AT_Message.h"

namespace cyclone
{
	class RingBuf;
}

namespace AT3
{

/** Axtrace Message Queue
*/
class MessageQueue
{
public:
	/** insert message to queue(should call by incoming thread)*/
	bool insertMessage(cyclone::RingBuf* buf, size_t msg_length, const LPSYSTEMTIME tTime);
	/** process message in quueu(should call by main thread)*/
	void processMessage(MessageVector& message);

private:
	Message* _popMessage(void);

private:
	enum { DEFAULT_RINGBUF_SIZE = 2048, MAX_RINGBUF_SIZE = 1024*1024*8 };
	CRITICAL_SECTION	m_criticalSection;
	HANDLE				m_hNotEmptySignal;
	cyclone::RingBuf*	m_ring_buf;

public:
	MessageQueue();
	virtual ~MessageQueue();
};

}
