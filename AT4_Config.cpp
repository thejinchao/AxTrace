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
		"\tend; \r\n"
		"\tif(logType==AXT_FATAL) then \r\n"
		"\t\tfrontColor=COL_RED; \r\n"
		"\t\tbackColor=COL_YELLOW; \r\n"
		"\tend; \r\n"
		"\treturn true, \"defult\", frontColor, backColor; \r\n"
		"end\n\r\n"
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
}

//--------------------------------------------------------------------------------------------
void Config::copyFrom(const Config& other)
{
	m_bCapture = other.m_bCapture;
	m_bAutoScroll = other.m_bAutoScroll;
	m_filterScript = other.m_filterScript;
	m_maxLogCounts = other.m_maxLogCounts;
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
void Config::loadSetting(void)
{
	_resetToDefaultSetting();

	//Load setting from disk
	QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

	m_bAutoScroll = settings.value("AutoScroll", m_bAutoScroll).toBool();
	m_bShowGrid = settings.value("ShowGrid", m_bShowGrid).toBool();
	m_mainGeometry = settings.value("MainGeometry", QByteArray()).toByteArray();
	m_filterScript = settings.value("FilterScript", m_filterScript).toString();
	m_maxLogCounts = settings.value("MaxLogCounts", m_maxLogCounts).toInt();
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
}
