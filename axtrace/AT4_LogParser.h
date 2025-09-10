/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
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

class LogParser;
typedef QSharedPointer<LogParser> LogParserPtr;
typedef QVector<LogParserPtr> LogParserVector;

class LogParser
{
public:
	struct Column
	{
		QString name;
		qint32 width;
	};
	typedef QVector<Column> ColumnVector;

	struct Define
	{
		QString titleRegular;
		QString logRegular;
		ColumnVector columns;
	};

	static bool tryLoadParserScript(const QString& script, QString& errorMsg, LogParserVector& logParserVector);

public:
	bool isDefault(void) const { 
		return m_parserDefine.logRegular.isEmpty();
	}

	bool isTitleMatch(const QString& title) const;

	const ColumnVector& getColumns(void) const { 
		return m_parserDefine.columns; 
	}

	QStringList parserLog(const QString& logContent) const;

private:
	Define m_parserDefine;
	QRegularExpression m_titleRegular;
	QRegularExpression m_logRegular;

public:
	LogParser();
	LogParser(const Define& parserDefine);
};
