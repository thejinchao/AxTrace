/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include "AT4_LogData.h"

class LogColumn
{
public:
	qint32 getIndex(void) const { return m_index; }

	qint32 getActiveIndex(void) const { return m_activeIndex; }
	bool isActive(void) const { return m_active; }

	virtual qint32 getWidth(void) const = 0;
	virtual QString getTitle(void) const = 0;
	virtual QString getString(const LogData& logData) const  = 0;

protected:
	const qint32 m_index;
	bool m_active;
	qint32 m_activeIndex;

public:
	LogColumn(qint32 index, bool active = true) : m_index(index), m_active(active), m_activeIndex(-1){ }
	friend class LogColumnGroup;
};

class LogColumn_Index : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 40; }
	virtual QString getTitle(void) const { return QString("#"); }
	virtual QString getString(const LogData& logData) const;

public:
	LogColumn_Index(qint32 index, bool active = true) : LogColumn(index, active) {}
};

class LogColumn_Time : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 90; }
	virtual QString getTitle(void) const { return QString("Time"); }
	virtual QString getString(const LogData& logData) const;

public:
	LogColumn_Time(qint32 index, bool active = true) : LogColumn(index, active) {}
};

class LogColumn_ProcessID : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 50; }
	virtual QString getTitle(void) const { return QString("PID"); }
	virtual QString getString(const LogData& logData) const;

public:
	LogColumn_ProcessID(qint32 index, bool active = true) : LogColumn(index, active) {}
};

class LogColumn_ThreadID : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 50; }
	virtual QString getTitle(void) const { return QString("TID"); }
	virtual QString getString(const LogData& logData) const;
public:
	LogColumn_ThreadID(qint32 index, bool active = true) : LogColumn(index, active) {}
};

class LogColumn_SessionName : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 100; }
	virtual QString getTitle(void) const { return QString("Session"); }
	virtual QString getString(const LogData& logData) const;
public:
	LogColumn_SessionName(qint32 index, bool active = true) : LogColumn(index, active) {}
};

class LogColumn_LogType : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 50; }
	virtual QString getTitle(void) const { return QString("Type"); }
	virtual QString getString(const LogData& logData) const;
public:
	LogColumn_LogType(qint32 index, bool active = true) : LogColumn(index, active) {}
};

class LogColumn_LogContent : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 0; }
	virtual QString getTitle(void) const { return QString("Log"); }
	virtual QString getString(const LogData& logData) const;
public:
	LogColumn_LogContent(qint32 index, bool active = true) : LogColumn(index, active) {}
};

typedef QVector< LogColumn* > LogColumnVector;

class LogColumnGroup
{
public:
	void initDefaulGroup(void);

	void activeColumn(qint32 index, bool active);
	qint32 getCounts(void) const { return m_columns.size(); }
	qint32 getActiveCounts(void) const { return m_activeCounts; }

	const LogColumn* getColumn(qint32 index) const;
	const LogColumn* getActiveColumn(qint32 activeIndex) const;

	typedef std::function<void(const LogColumn*)> ColumnWalkFunc;
	void walk(bool activeOnly, ColumnWalkFunc walkFunc) const;

private:
	void _resetActiveIndex(void);

private:
	LogColumnVector m_columns;
	qint32 m_activeCounts;

public:
	LogColumnGroup() : m_activeCounts(0) { }
	~LogColumnGroup() { }
};
