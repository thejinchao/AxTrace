/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include "AT4_Interface.h"
#include "AT4_Session.h"
#include "AT4_Config.h"

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

class LogParser
{
public:
	bool isDefault(void) const { return m_parserDefine->regExp.isEmpty(); }

	typedef Config::LogWndColumnVector ColumnVector;
	const ColumnVector& getTitleList(void) const { return m_parserDefine->columns; }

	QStringList parserLog(const QString& logContent) const;

private:
	Config::LogParserDefinePtr	m_parserDefine;
	QRegExp m_regExp;

public:
	LogParser(const Config::LogParserDefinePtr logParserDefine);
};
