/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include "AT4_Message.h"

class MessageQueue
{
public:
	/** insert message to queue(should call by incoming thread)*/
	void insertMessage(cyclone::RingBuf* buf, size_t msg_length, const QTime& tTime);
	/** pop message to queue(should call by main thread)*/
	void popMessage(MessageVector& message);
	/** clean all message directorly */
	void cleanMessage(void);

private:
	Message* _popMessage(void);

private:
	enum { DEFAULT_RINGBUF_SIZE = 2048, MAX_RINGBUF_SIZE = 1024 * 1024 * 8 };
	cyclone::RingBuf*	m_ring_buf;
	QMutex				m_lock;
	QAtomicInt			m_counts;

public:
	MessageQueue();
	virtual ~MessageQueue();
};
