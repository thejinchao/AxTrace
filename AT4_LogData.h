/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include "AT4_Interface.h"
#include "AT4_Session.h"

struct LogData
{
	quint32			logIndex;
	axtrace_time_s	logTime;
	SessionPtr		session;
	quint32			logType;
	QString			logContent;
	QColor			backColor;
	QColor			frontColor;
};

typedef QQueue<LogData> LogDataVector;
