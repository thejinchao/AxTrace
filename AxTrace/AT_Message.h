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
	static const char* MESSAGE_META_NAME;

	/** build message */
	virtual void build(const AXIATRACE_TIME& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf) = 0;
	/** get trace type*/
	virtual unsigned int getTraceType(void) const = 0;
	/** get process id*/
	unsigned int getProcessID(void) const { return m_nProcessID; }
	/** get thread id*/
	unsigned int getThreadID(void) const { return m_nThreadID; }
	/** get Style id*/
	unsigned int getStyleID(void) const { return m_nStyleID; }
	/** get trace time */
	const AXIATRACE_TIME* getTraceTime(void) const { return &m_traceTime; }

public:
	static void _luaopen(lua_State *L);

protected:
	unsigned int	m_nProcessID;
	unsigned int	m_nThreadID;
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
	virtual unsigned int getTraceType(void) const { return AXTRACE_CMD_TYPE_LOG; }

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

/** Graphics2D Init/Clean Map Message
*/
class G2DCleanMapMessage : public Message
{
public:
	/** build message */
	virtual void build(const AXIATRACE_TIME& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf);
	/** get trace type*/
	virtual unsigned int getTraceType(void) const { return AXTRACE_CMD_TYPE_2D_CLEAN_MAP; }

	const char* getMapName(void) const { return m_map_name; }
	double get_x_size(void) const { return x_size; }
	double get_y_size(void) const { return y_size; }

private:
	char	m_map_name[AXTRACE_MAX_MAP_NAME_LENGTH];
	double	x_size;
	double	y_size;

public:
	G2DCleanMapMessage();
	virtual ~G2DCleanMapMessage();
};


/** Graphics2D Create/Update Actor Message
*/
class G2DActorMessage : public Message
{
public:
	/** build message */
	virtual void build(const AXIATRACE_TIME& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf);
	/** get trace type*/
	virtual unsigned int getTraceType(void) const { return AXTRACE_CMD_TYPE_2D_ACTOR; }

	const char* getMapName(void) const { return m_map_name; }
	unsigned int get_id(void) const { return actor_id; }
	double get_x(void) const { return x_pos; }
	double get_y(void) const { return y_pos; }
	double get_dir(void) const { return dir; }

private:
	char	m_map_name[AXTRACE_MAX_MAP_NAME_LENGTH];
	unsigned int actor_id;
	double	x_pos;
	double	y_pos;
	double	dir;

public:
	G2DActorMessage();
	virtual ~G2DActorMessage();
};

typedef std::vector< Message* > MessageVector;

}
