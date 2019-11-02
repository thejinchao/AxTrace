/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"

#include "AT4_LogColumn.h"

//--------------------------------------------------------------------------------------------
QString LogColumn_Index::getString(const LogData& logData) const
{
	return QString("%1").arg(logData.logIndex);
}

//--------------------------------------------------------------------------------------------
QString LogColumn_Time::getString(const LogData& logData) const
{
	const MessageTime& t = logData.logTime;
	return  QString("%1:%2 %3.%4")
		.arg(t.hour, 2, 10, QLatin1Char('0'))
		.arg(t.minute, 2, 10, QLatin1Char('0'))
		.arg(t.second, 2, 10, QLatin1Char('0'))
		.arg(t.milliseconds, 3, 10, QLatin1Char('0'));
}

//--------------------------------------------------------------------------------------------
QString LogColumn_ProcessID::getString(const LogData& logData) const
{
	return QString("%1").arg(logData.session->getProcessID());
}

//--------------------------------------------------------------------------------------------
QString LogColumn_ThreadID::getString(const LogData& logData) const
{
	return QString("%1").arg(logData.session->getThreadID());
}

//--------------------------------------------------------------------------------------------
QString LogColumn_SessionName::getString(const LogData& logData) const
{
	return logData.session->getSessionName();
}

//--------------------------------------------------------------------------------------------
QString LogColumn_LogType::getString(const LogData& logData) const
{
	static QString sStandardLogType[] =
	{
		"Trace", "Debug","Info","Warn","Error","Fatal"
	};
	const static int sStandardLogTypeCounts = 6;

	if (logData.logType < sStandardLogTypeCounts)
	{
		return sStandardLogType[logData.logType];
	}

	return QString("%1").arg(logData.logType);
}

//--------------------------------------------------------------------------------------------
QString LogColumn_LogContent::getString(const LogData& logData) const
{
	return logData.logContent;
}

//--------------------------------------------------------------------------------------------
void LogColumnGroup::initDefaulGroup(void)
{
	int index = 0;

	m_columns.push_back(new LogColumn_Index(index++));
	m_columns.push_back(new LogColumn_Time(index++));
	m_columns.push_back(new LogColumn_LogType(index++, false));
	m_columns.push_back(new LogColumn_SessionName(index++, false));
	m_columns.push_back(new LogColumn_ProcessID(index++, false));
	m_columns.push_back(new LogColumn_ThreadID(index++, false));
	m_columns.push_back(new LogColumn_LogContent(index++));

	_resetActiveIndex();
}

//--------------------------------------------------------------------------------------------
void LogColumnGroup::activeColumn(qint32 index, bool active)
{
	Q_ASSERT(index >= 0 && index < m_columns.size());

	m_columns[index]->m_active = active;
	_resetActiveIndex();
}

//--------------------------------------------------------------------------------------------
const LogColumn* LogColumnGroup::getColumn(qint32 index) const
{
	Q_ASSERT(index >= 0 && index < m_columns.size());

	return m_columns[index];
}

//--------------------------------------------------------------------------------------------
const LogColumn* LogColumnGroup::getActiveColumn(qint32 activeIndex) const
{
	Q_ASSERT(activeIndex >= 0 && activeIndex < m_activeCounts);
	
	for (LogColumn* column : m_columns)
	{
		if (column->isActive() && column->getActiveIndex() == activeIndex)
		{
			return column;
		}
	}
	Q_ASSERT(false);
	return nullptr;
}

//--------------------------------------------------------------------------------------------
void LogColumnGroup::_resetActiveIndex(void)
{
	m_activeCounts = 0;
	for (LogColumn* column : m_columns)
	{
		if (column->isActive())
		{
			column->m_activeIndex = m_activeCounts++;
		}
		else
			column->m_activeIndex = m_activeCounts;
	}
}

//--------------------------------------------------------------------------------------------
void LogColumnGroup::walk(bool activeOnly, ColumnWalkFunc walkFunc) const
{
	for (LogColumn* column : m_columns)
	{
		if (activeOnly && column->isActive() || !activeOnly)
		{
			walkFunc(column);
		}
	}
}
