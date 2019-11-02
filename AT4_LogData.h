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
	MessageTime		logTime;
	SessionPtr		session;
	quint32			logType;
	QColor			backColor;
	QColor			frontColor;
	QString			logContent;
};

typedef QQueue<LogData> LogDataVector;
