/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#include "stdafx.h"
#include "AT4_Config.h"
#include "AT4_System.h"
#include "AT4_MainWindow.h"

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
		"\tlocal shape = ACTOR_CIRCLE;\r\n"
		"\tlocal actor_style = msg:get_actor_style();\r\n"
		"\tif (actor_style == 0) then\r\n"
		"\t\treturn true, ACTOR_TRIANGLE, 5, COL_WHITE, COL_GRAY;\r\n"
		"\telseif(actor_style == 1) then\r\n"
		"\t\treturn true, ACTOR_QUAD, 5, COL_WHITE, COL_GRAY;\r\n"
		"\telse\r\n"
		"\t\treturn true, ACTOR_CIRCLE, 5, COL_WHITE, COL_RED;\r\n"
		"\tend;\r\n"
		"end; \r\n"
	);
	m_maxLogCounts = MAX_LOG_COUNTS_DEFAULT;

	m_bShowTail = true;
	m_maxActorTailCounts = MAX_ACTOR_TAIL_COUNTS_DEFAULT;

	m_defaultLogParserDefineScript = m_logParserDefineScript = QString(
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

	m_defaultLogParser = QSharedPointer<LogParser>::create();

	m_maxActorLogCounts = MAX_ACTOR_LOG_COUNTS_DEFAULT;

	m_listenPort = LISTEN_PORT_DEFAULT;
}

//--------------------------------------------------------------------------------------------
void Config::copyFrom(const Config& other)
{
	m_bCapture = other.m_bCapture;
	m_bAutoScroll = other.m_bAutoScroll;
	m_filterScript = other.m_filterScript;
	m_maxLogCounts = other.m_maxLogCounts;
	m_logParserDefineScript = other.m_logParserDefineScript;
	m_logParserVector = other.m_logParserVector;
	m_maxActorLogCounts = other.m_maxActorLogCounts;
	m_listenPort = other.m_listenPort;
	m_bShowTail = other.m_bShowTail;
	m_maxActorTailCounts = other.m_maxActorTailCounts;
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
	Q_ASSERT(maxLogCounts > MAX_LOG_COUNTS_RANGE_MIN && maxLogCounts < MAX_LOG_COUNTS_RANGE_MAX);

	m_maxLogCounts = maxLogCounts;
}

//--------------------------------------------------------------------------------------------
void Config::setMaxActorLogCounts(qint32 maxActorLogCounts)
{
	Q_ASSERT(maxActorLogCounts > MAX_ACTOR_LOG_COUNTS_RANGE_MIN && maxActorLogCounts < MAX_ACTOR_LOG_COUNTS_RANGE_MAX);

	m_maxActorLogCounts = maxActorLogCounts;
}

//--------------------------------------------------------------------------------------------
void Config::setMaxActorTailCounts(qint32 maxActorTailCounts)
{
	Q_ASSERT(maxActorTailCounts > MAX_ACTOR_TAIL_COUNTS_RANGE_MIN && maxActorTailCounts < MAX_ACTOR_TAIL_COUNTS_RANGE_MAX);

	m_maxActorTailCounts = maxActorTailCounts;
}

//--------------------------------------------------------------------------------------------
void Config::setListenPort(qint32 listenPort)
{
	Q_ASSERT(listenPort >= LISTEN_PORT_MIN && listenPort<= LISTEN_PORT_MAX);
	Q_ASSERT(System::getSingleton()->getSessionManager()->getSessionCounts() == 0);

	m_listenPort = listenPort;
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
	m_maxActorLogCounts = settings.value("MaxActorLogCounts", m_maxActorLogCounts).toInt();
	m_listenPort = settings.value("ListenPort", m_listenPort).toInt();
	m_bShowTail = settings.value("ShowTail", m_bShowTail).toBool();
	m_maxActorTailCounts = settings.value("MaxActorTailCounts", m_maxActorTailCounts).toInt();
	//load log parser define
	QString logParserDefineScript = settings.value("LogParserScript", m_defaultLogParserDefineScript).toString();

	QString errorMsg;
	if (!LogParser::tryLoadParserScript(logParserDefineScript, errorMsg, m_logParserVector))
	{
		return false;
	}
	m_logParserDefineScript = logParserDefineScript;
	return true;
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
	settings.setValue("LogParserScript", m_logParserDefineScript);
	settings.setValue("MaxActorLogCounts", m_maxActorLogCounts);
	settings.setValue("ListenPort", m_listenPort);
	settings.setValue("ShowTail", m_bShowTail);
	settings.setValue("MaxActorTailCounts", m_maxActorTailCounts);
}

//--------------------------------------------------------------------------------------------
const LogParserPtr Config::getLogParser(const QString& title) const
{
	LogParserVector::const_iterator it;
	
	for (it = m_logParserVector.begin(); it != m_logParserVector.end(); it++)
	{
		if ((*it)->isTitleMatch(title)) 
		{
			return *it;
		}
	}
	return m_defaultLogParser;
}

//--------------------------------------------------------------------------------------------
void Config::setLogParserScript(const QString& logParserScript)
{
	Q_ASSERT(System::getSingleton()->getMainWindow()->getLogChildCounts() == 0);

	QString errorMsg;
	LogParserVector logParserVector;
	bool success = LogParser::tryLoadParserScript(logParserScript, errorMsg, logParserVector);
	Q_ASSERT(success);
	if (!success)
	{
		return;
	}

	m_logParserDefineScript = logParserScript;
	m_logParserVector = logParserVector;
}
