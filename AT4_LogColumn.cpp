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
	const axtrace_time_s& t = logData.logTime;
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
