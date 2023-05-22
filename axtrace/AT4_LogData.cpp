/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#include "stdafx.h"

#include "AT4_LogData.h"

//--------------------------------------------------------------------------------------------
LogParser::LogParser(const DefinePtr parserDefine)
	: m_parserDefine(parserDefine)
{
	if (isDefault()) return;

	m_regExp = QRegularExpression(m_parserDefine->regExp);
	Q_ASSERT(m_regExp.captureCount() == m_parserDefine->columns.size());
}

//--------------------------------------------------------------------------------------------
bool LogParser::tryLoadParserScript(const QString& script, QString& errorMsg, DefineMap& logParserDefineMap)
{
	QJsonParseError json_error;
	QJsonDocument jsonDoc(QJsonDocument::fromJson(script.toUtf8(), &json_error));

	if (json_error.error != QJsonParseError::NoError)
	{
		errorMsg = json_error.errorString();
		return false;
	}

	DefineMap defineMap;
	QJsonArray arrayObj = jsonDoc.array();
	for (int i = 0; i < arrayObj.size(); i++)
	{
		DefinePtr parser(new Define());

		QJsonObject obj = arrayObj.at(i).toObject();

		parser->title = obj.value("title").toString();
		parser->regExp = obj.value("regex").toString();

		if (parser->title == "") {
			errorMsg = QString("Can't find 'title' in the parser %1").arg(i);
			return false;
		}

		if (parser->regExp == "") {
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
			parser->columns.push_back(newColumn);
		}

		QRegularExpression regExp(parser->regExp);
		if (regExp.captureCount() != parser->columns.size())
		{
			errorMsg = QString("The capture counts is %1, but column counts is %2 in the parser %3 ")
				.arg(regExp.captureCount())
				.arg(parser->columns.size())
				.arg(i);
			return false;
		}

		defineMap.insert(parser->title, parser);
	}

	logParserDefineMap = defineMap;
	return true;
}

//--------------------------------------------------------------------------------------------
QStringList LogParser::parserLog(const QString& logContent) const
{
	if (isDefault())
	{
		return QStringList(logContent);
	}
	
	QRegularExpressionMatch match = m_regExp.match(logContent);
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
