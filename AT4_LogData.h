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
	QStringList		logContent;
};

typedef QQueue<LogData> LogDataVector;

class LogParser
{
public:
	bool build(const QString& regString, const QStringList& titleList);

	bool isDefault(void) const { return m_default; }
	const QStringList& getTitleList(void) const { return m_titleList; }
	QStringList parserLog(const QString& logContent) const;

private:
	void _resetToDefault(void);

private:
	bool		m_default;
	QRegExp		m_regExp;
	QStringList	m_titleList;

public:
	LogParser();
};
