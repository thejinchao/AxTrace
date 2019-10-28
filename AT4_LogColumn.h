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
	virtual qint32 getWidth(void) const = 0;
	virtual QString getTitle(void) const = 0;
	virtual QString getString(const LogData& logData) const  = 0;

protected:
	qint32 m_index;

public:
	LogColumn(qint32 index) : m_index(index) {}
};

class LogColumn_Index : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 40; }
	virtual QString getTitle(void) const { return QString("#"); }
	virtual QString getString(const LogData& logData) const;

public:
	LogColumn_Index(qint32 index) : LogColumn(index) {}
};

class LogColumn_Time : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 90; }
	virtual QString getTitle(void) const { return QString("Time"); }
	virtual QString getString(const LogData& logData) const;

public:
	LogColumn_Time(qint32 index) : LogColumn(index) {}
};

class LogColumn_ProcessID : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 50; }
	virtual QString getTitle(void) const { return QString("PID"); }
	virtual QString getString(const LogData& logData) const;

public:
	LogColumn_ProcessID(qint32 index) : LogColumn(index) {}
};

class LogColumn_ThreadID : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 50; }
	virtual QString getTitle(void) const { return QString("TID"); }
	virtual QString getString(const LogData& logData) const;
public:
	LogColumn_ThreadID(qint32 index) : LogColumn(index) {}
};

class LogColumn_SessionName : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 100; }
	virtual QString getTitle(void) const { return QString("Name"); }
	virtual QString getString(const LogData& logData) const;
public:
	LogColumn_SessionName(qint32 index) : LogColumn(index) {}
};

class LogColumn_LogType : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 50; }
	virtual QString getTitle(void) const { return QString("Type"); }
	virtual QString getString(const LogData& logData) const;
public:
	LogColumn_LogType(qint32 index) : LogColumn(index) {}
};

class LogColumn_LogContent : public LogColumn
{
public:
	virtual qint32 getWidth(void) const { return 0; }
	virtual QString getTitle(void) const { return QString("Log"); }
	virtual QString getString(const LogData& logData) const;
public:
	LogColumn_LogContent(qint32 index) : LogColumn(index) {}
};

typedef QVector< LogColumn* > LogColumnVector;
