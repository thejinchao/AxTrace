/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_Config.h"

//--------------------------------------------------------------------------------------------
Config::Config()
{
	// reset setting to default
	_resetToDefaultSetting();
}

//--------------------------------------------------------------------------------------------
Config::Config(const Config& other)
{
	// reset setting to default
	_resetToDefaultSetting();
	copyFrom(other);
}

//--------------------------------------------------------------------------------------------
Config::~Config()
{
	saveSetting();
}

//--------------------------------------------------------------------------------------------
void Config::_resetToDefaultSetting(void)
{
	m_bCapture = true;
	m_bAutoScroll = true;
	m_bShowGrid = true;
	m_filterScript = m_defaultFilterScript = QString(
		"--[[ \r\n"
		"msg:get_type(), msg:get_pid(), msg:get_tid() \r\n"
		"--]] \r\n"
		"----------- \r\n"
		"--[[ \r\n"
		"msg:get_log_type(), msg:get_log() \r\n"
		"--]] \r\n"
		"function onLogMessage(msg)\r\n"
		"\tlocal frontColor=COL_BLACK; \r\n"
		"\tlocal backColor=COL_WHITE; \r\n"
		"\tlocal logType=msg:get_log_type(); \r\n"
		"\tif(logType==AXT_ERROR) then \r\n"
		"\t\tfrontColor=COL_RED; \r\n"
		"\telseif(logType==AXT_FATAL) then \r\n"
		"\t\tfrontColor=COL_RED; \r\n"
		"\t\tbackColor=COL_YELLOW; \r\n"
		"\telseif(logType==AXT_USERDEF+1) then \r\n"
		"\t\treturn true, \"Talk\", frontColor, backColor; \r\n"
		"\tend; \r\n"
		"\treturn true, \"defult\", frontColor, backColor; \r\n"
		"end;\n\r\n"
		"--[[ \r\n"
		"msg:get_value() \r\n"
		"--]] \r\n"
		"function onValueMessage(msg) \r\n"
		"\treturn true, \"defult\", COL_BLACK, COL_WHITE; \r\n"
		"end; \r\n\r\n"
		"--[[ \r\n"
		"msg:get_actor_id(), msg:get_actor_position(), msg:get_actor_dir(), msg:get_actor_style() \r\n"
		"--]] \r\n"
		"function onActor2DMessage(msg)\r\n"
		"\t\treturn true, ACTOR_CIRCLE, 1, COL_WHITE, COL_GRAY; \r\n"
		"end; \r\n"
	);
	m_maxLogCounts = 10000;

	m_defaultLogParserDefineString = m_logParserDefineString = QString(
		"[\r\n"
		"  {\r\n"
		"    \"title\": \"Talk\",\r\n"
		"    \"regex\": \"(.*)->(.*):(.*)\",\r\n"
		"    \"column\": [\r\n"
		"      {\r\n"
		"        \"name\": \"Sender\",\r\n"
		"        \"width\": \"100\"\r\n"
		"      },\r\n"
		"      {\r\n"
		"        \"name\": \"Receiver\",\r\n"
		"        \"width\": \"100\"\r\n"
		"      },\r\n"
		"      {\r\n"
		"        \"name\": \"Content\",\r\n"
		"        \"width\": \"0\"\r\n"
		"      }\r\n"
		"    ]\r\n"
		"  }\r\n"
		"]\r\n"
	);

	m_emptyLogParser = LogParserDefinePtr(new LogParserDefine());
	m_emptyLogParser->columns.push_back({"Log", 0});
}

//--------------------------------------------------------------------------------------------
void Config::copyFrom(const Config& other)
{
	m_bCapture = other.m_bCapture;
	m_bAutoScroll = other.m_bAutoScroll;
	m_filterScript = other.m_filterScript;
	m_maxLogCounts = other.m_maxLogCounts;
	m_logParserDefineString = other.m_logParserDefineString;
	m_logParserDefineMap = other.m_logParserDefineMap;
}

//--------------------------------------------------------------------------------------------
void Config::setFilterScript(const QString& script)
{
	m_filterScript = script;
	saveSetting();
}

//--------------------------------------------------------------------------------------------
void Config::setMainGeometry(const QByteArray& geometry)
{
	m_mainGeometry = geometry;
}

//--------------------------------------------------------------------------------------------
void Config::setMaxLogCounts(int maxLogCounts)
{
	m_maxLogCounts = maxLogCounts;
}

//--------------------------------------------------------------------------------------------
bool Config::loadSetting(void)
{
	_resetToDefaultSetting();

	//Load setting from disk
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

	m_bAutoScroll = settings.value("AutoScroll", m_bAutoScroll).toBool();
	m_bShowGrid = settings.value("ShowGrid", m_bShowGrid).toBool();
	m_mainGeometry = settings.value("MainGeometry", QByteArray()).toByteArray();
	m_filterScript = settings.value("FilterScript", m_filterScript).toString();
	m_maxLogCounts = settings.value("MaxLogCounts", m_maxLogCounts).toInt();
	m_logParserDefineString = settings.value("LogParserDefine", m_defaultLogParserDefineString).toString();

	//load log parser define
	return _loadLogParserDefine();
}

//--------------------------------------------------------------------------------------------
void Config::saveSetting(void) const
{
	// Save setting to disk
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

	settings.setValue("AutoScroll", m_bAutoScroll);
	settings.setValue("ShowGrid", m_bShowGrid);
	settings.setValue("MainGeometry", m_mainGeometry);
	settings.setValue("FilterScript", m_filterScript);
	settings.setValue("MaxLogCounts", m_maxLogCounts);
	settings.setValue("LogParserDefine", m_logParserDefineString);
}

//--------------------------------------------------------------------------------------------
bool Config::_loadLogParserDefine(void)
{
	QJsonParseError json_error;
	QJsonDocument jsonDoc(QJsonDocument::fromJson(m_logParserDefineString.toUtf8(), &json_error));

	if (json_error.error != QJsonParseError::NoError)
	{
		return false;
	}

	QJsonArray arrayObj = jsonDoc.array();
	for (int i = 0; i < arrayObj.size(); i++)
	{
		LogParserDefinePtr parser(new LogParserDefine());

		QJsonObject obj = arrayObj.at(i).toObject();

		parser->title = obj.value("title").toString();
		parser->regExp = obj.value("regex").toString();

		QJsonArray columnArray = obj.value("column").toArray();
		for (int j = 0; j < columnArray.size(); j++)
		{
			LogWndColumn newColumn;
			QJsonObject columnObj = columnArray.at(j).toObject();

			newColumn.name = columnObj.value("name").toString();
			newColumn.width = columnObj.value("width").toString().toInt();

			parser->columns.push_back(newColumn);
		}

		m_logParserDefineMap.insert(parser->title, parser);
	}

	return true;
}

//--------------------------------------------------------------------------------------------
const Config::LogParserDefinePtr Config::getLogParser(const QString& title) const
{
	LogParserDefineMap::const_iterator it = m_logParserDefineMap.find(title);
	if (it == m_logParserDefineMap.end())
	{
		return m_emptyLogParser;
	}

	return it.value();
}
