/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"

#include "AT4_LogData.h"

//--------------------------------------------------------------------------------------------
LogParser::LogParser(const Config::LogParserDefinePtr logParserDefine)
	: m_parserDefine(logParserDefine)
{
	if (isDefault()) return;

	m_regExp = QRegExp(m_parserDefine->regExp);
	Q_ASSERT(m_regExp.captureCount() == m_parserDefine->columns.size());
}

//--------------------------------------------------------------------------------------------
QStringList LogParser::parserLog(const QString& logContent) const
{
	if (isDefault() || !m_regExp.exactMatch(logContent))
	{
		return QStringList(logContent);
	}

	QStringList ret;
	QStringList captureTexts = m_regExp.capturedTexts();
	for (qint32 i = 1; i < captureTexts.size(); i++)
	{
		ret << captureTexts[i];
	}
	return ret;
}
