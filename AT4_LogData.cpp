/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"

#include "AT4_LogData.h"

//--------------------------------------------------------------------------------------------
LogParser::LogParser()
{
	_resetToDefault();
}

//--------------------------------------------------------------------------------------------
void LogParser::_resetToDefault(void)
{
	m_default = true;
	m_titleList = (QStringList() << "Log");
}

//--------------------------------------------------------------------------------------------
bool LogParser::build(const QString& regString, const QStringList& titleList)
{
	_resetToDefault();

	if (regString.isEmpty())
	{
		return true;
	}

	QRegExp regExp(regString);
	if (regExp.captureCount() != titleList.size())
	{
		return false;
	}

	m_default = false;
	m_regExp = regExp;
	m_titleList = titleList;
	return true;
}

//--------------------------------------------------------------------------------------------
QStringList LogParser::parserLog(const QString& logContent) const
{
	if (!isDefault())
	{
		if (m_regExp.exactMatch(logContent))
		{
			QStringList ret;
			QStringList captureTexts = m_regExp.capturedTexts();
			for (qint32 i = 1; i < captureTexts.size(); i++)
			{
				ret << captureTexts[i];
			}
			return ret;
		}
	}

	return QStringList(logContent);
}
