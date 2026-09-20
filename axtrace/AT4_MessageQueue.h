/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#pragma once

#include "AT4_Message.h"

class MessageQueue
{
public:
	/** insert message to queue(should call by incoming thread)*/
	void insertMessage(cyclone::RingBuf& sourceBuf, size_t msgLength, const QDateTime& tTime, qint32 sessionID);
	/** pop message to queue(should call by main thread)*/
	void popMessage(MessageVector& message);

private:
	Message* _popMessage(void);

private:
	enum { DEFAULT_RINGBUF_SIZE = 2048 };

	cyclone::RingBuf	m_ringBuf;
	QMutex				m_lock;
	QAtomicInt			m_msgCountsInBuff;
	QByteArray			m_tempBuf;
public:
	MessageQueue();
	virtual ~MessageQueue();
};
