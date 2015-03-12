/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once

#include "AT_Interface.h"

namespace cyclone
{
	class RingBuf;
}

namespace AT3
{
/** Axtrace Message base class
*/
class Message
{
public:
	/** build message */
	virtual void build(const AXIATRACE_TIME& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf) = 0;
	/** get trace type*/
	virtual unsigned int getTraceType(void) const = 0;
	/** get process id*/
	unsigned int getProcessID(void) const { return m_nProcessID; }
	/** get thread id*/
	unsigned int getThreadID(void) const { return m_nThreadID; }
	/** get window id*/
	unsigned int getWindowID(void) const { return m_nWindowID; }
	/** get Style id*/
	unsigned int getStyleID(void) const { return m_nStyleID; }
	/** get trace time */
	const AXIATRACE_TIME* getTraceTime(void) const { return &m_traceTime; }

protected:
	unsigned int	m_nProcessID;
	unsigned int	m_nThreadID;
	unsigned int	m_nWindowID;
	unsigned int	m_nStyleID;
	AXIATRACE_TIME	m_traceTime;

public:
	Message();
	virtual ~Message();
};

/** Log message
*/
class LogMessage : public Message
{
public:
	/** build message */
	virtual void build(const AXIATRACE_TIME& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf);
	/** get trace type*/
	virtual unsigned int getTraceType(void) const { return AXTRACE_CMD_TYPE_TRACE; }

	/** get log size*/
	size_t getLogSizeChar(void) const { return m_pLogBufInChar; }
	/** get content buf*/
	const wchar_t* getLogBuf(void) const { return m_pLogBuf; }

private:
	wchar_t*	m_pLogBuf;
	size_t		m_pLogBufInChar;

public:
	LogMessage();
	virtual ~LogMessage();
};

/** Value Message
*/
class ValueMessage : public Message
{
public:
	/** build message */
	virtual void build(const AXIATRACE_TIME& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf);
	/** get trace type*/
	virtual unsigned int getTraceType(void) const { return AXTRACE_CMD_TYPE_VALUE; }

	/** get name */
	const wchar_t* getValueName(void) const { return m_name; }
	/** get value as string */
	void getValueAsString(std::wstring& value) const;

private:
	unsigned int m_valuetype;
	wchar_t	m_name[AXTRACE_MAX_VALUENAME_LENGTH];

	void*	m_valueBuf;
	size_t	m_valueSize;

	enum { STANDARD_VALUE_SIZE =32 };
	unsigned char m_standValueBuf[STANDARD_VALUE_SIZE];

public:
	ValueMessage();
	virtual ~ValueMessage();
};

typedef std::vector< Message* > MessageVector;

}
