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
	struct Column
	{
		QString name;
		qint32 width;
	};
	typedef QVector<Column> ColumnVector;

	struct Define
	{
		QString title;
		QString regExp;
		ColumnVector columns;
	};
	typedef QSharedPointer<Define> DefinePtr;
	typedef QHash<QString, DefinePtr> DefineMap;

	static bool tryLoadParserScript(const QString& script, QString& errorMsg, DefineMap& defineMap);

public:
	bool isDefault(void) const { 
		return m_parserDefine->regExp.isEmpty(); 
	}

	const ColumnVector& getColumns(void) const { 
		return m_parserDefine->columns; 
	}

	QStringList parserLog(const QString& logContent) const;

private:
	DefinePtr m_parserDefine;
	QRegularExpression m_regExp;

public:
	LogParser(const DefinePtr parserDefine);
};
