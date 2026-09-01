/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
***************************************************/
#pragma once

#include "Session/AT4_Session.h"
#include "Data/AT4_CommonStruct.h"

struct LogData
{
	quint32			logIndex;
	MessageTime		logTime;
	SessionPtr		session;
	quint32			logType;
	QColor			backColor;
	QColor			frontColor;
	QStringList		logContent;
};

typedef QQueue<LogData> LogDataVector;

struct LogFilterResult
{
	bool		display;
	QString		wndTitle;
	uint16_t	fontColor;
	uint16_t	backColor;
};
