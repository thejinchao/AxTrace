/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once

extern "C"
{
#include "c-ringbuf/ringbuf.h"
}
#include "AT_Message.h"

namespace AT3
{

/** Axtrace Message Queue
*/
class MessageQueue
{
public:
	/** insert message to queue(should call by incoming thread)*/
	bool insertMessage(const char* pMessage, size_t size, const LPSYSTEMTIME tTime);
	/** process message in quueu(should call by main thread)*/
	void processMessage(MessageVector& message);

private:
	Message* _popMessage(void);
	bool _skipMessage(void);
	bool _checkMessageValid(const void* pMessage, size_t size);

private:
	enum { DEFAULT_RINGBUF_SIZE = 2048, MAX_RINGBUF_SIZE = 1024*1024*8 };
	ringbuf_t			m_ringBuf;
	CRITICAL_SECTION	m_criticalSection;
	HANDLE				m_hNotEmptySignal;

public:
	MessageQueue();
	virtual ~MessageQueue();
};

}
