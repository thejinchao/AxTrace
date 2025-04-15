/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#include "stdafx.h"

#include "AT4_LogData.h"

//--------------------------------------------------------------------------------------------
LogParser::LogParser()
{
	//default define
	m_parserDefine.titleRegular = "";
	m_parserDefine.logRegular = "";
	m_parserDefine.columns.push_back({ "Log", 0 });
}

//--------------------------------------------------------------------------------------------
LogParser::LogParser(const Define& parserDefine)
	: m_parserDefine(parserDefine)
{
	if (isDefault()) return;

	m_titleRegular = QRegularExpression(m_parserDefine.titleRegular);
	m_logRegular = QRegularExpression(m_parserDefine.logRegular);
	Q_ASSERT(m_logRegular.captureCount() == m_parserDefine.columns.size());
}

//--------------------------------------------------------------------------------------------
bool LogParser::tryLoadParserScript(const QString& script, QString& errorMsg, LogParserVector& logParserVector)
{
	QJsonParseError json_error;
	QJsonDocument jsonDoc(QJsonDocument::fromJson(script.toUtf8(), &json_error));

	if (json_error.error != QJsonParseError::NoError)
	{
		errorMsg = json_error.errorString();
		return false;
	}

	LogParserVector tempVector;
	QJsonArray arrayObj = jsonDoc.array();
	for (int i = 0; i < arrayObj.size(); i++)
	{
		Define parserDefine;

		QJsonObject obj = arrayObj.at(i).toObject();

		parserDefine.titleRegular = obj.value("title").toString();
		parserDefine.logRegular = obj.value("regex").toString();

		if (parserDefine.titleRegular == "") {
			errorMsg = QString("Can't find 'title' in the parser %1").arg(i);
			return false;
		}

		if (parserDefine.logRegular == "") {
			errorMsg = QString("Can't find 'regex' in the parser %1").arg(i);
			return false;
		}

		QJsonArray columnArray = obj.value("column").toArray();
		for (int j = 0; j < columnArray.size(); j++)
		{
			Column newColumn;
			QJsonObject columnObj = columnArray.at(j).toObject();

			newColumn.name = columnObj.value("name").toString();
			QString width = columnObj.value("width").toString();

			if (newColumn.name == "") {
				errorMsg = QString("Can't find 'name' in the parser %1 column %2").arg(i).arg(j);
				return false;
			}

			if (width == "") {
				errorMsg = QString("Can't find 'width' in the parser %1 column %2").arg(i).arg(j);
				return false;
			}

			newColumn.width = width.toInt();
			parserDefine.columns.push_back(newColumn);
		}

		QRegularExpression titleRegular(parserDefine.titleRegular);
		if (!titleRegular.isValid())
		{
			errorMsg = QString("The title regular is invalid");
			return false;
		}

		QRegularExpression logRegular(parserDefine.logRegular);
		if (logRegular.captureCount() != parserDefine.columns.size())
		{
			errorMsg = QString("The capture counts is %1, but column counts is %2 in the parser %3 ")
				.arg(logRegular.captureCount())
				.arg(parserDefine.columns.size())
				.arg(i);
			return false;
		}

		tempVector.push_back(QSharedPointer<LogParser>::create(parserDefine));
	}

	logParserVector = tempVector;
	return true;
}

//--------------------------------------------------------------------------------------------
QStringList LogParser::parserLog(const QString& logContent) const
{
	if (isDefault())
	{
		return QStringList(logContent);
	}
	
	QRegularExpressionMatch match = m_logRegular.match(logContent);
	if (!match.hasMatch())
	{
		return QStringList(logContent);
	}

	QStringList ret;
	QStringList captureTexts = match.capturedTexts();
	for (qint32 i = 1; i < captureTexts.size(); i++)
	{
		ret << captureTexts[i];
	}
	return ret;
}

//--------------------------------------------------------------------------------------------
bool LogParser::isTitleMatch(const QString& title) const 
{
	QRegularExpressionMatch isMatch = m_titleRegular.match(title);
	return isMatch.hasMatch();
}
