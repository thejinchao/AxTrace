/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include "AT4_LogData.h"

class Config
{
public:
	bool loadSetting(void);
	void saveSetting(void) const;
	void copyFrom(const Config& other);

	bool getCapture(void) const { return m_bCapture; }
	void setCapture(bool c) { m_bCapture = c; }

	bool getAutoScroll(void) const { return m_bAutoScroll; }
	void setAutoScroll(bool a) { m_bAutoScroll = a; }

	bool getShowGrid(void) const { return m_bShowGrid; }
	void setShowGrid(bool s) { m_bShowGrid = s; }

	const QString& getFilterScript(void) const { return m_filterScript; }
	void setFilterScript(const QString& script);

	const QByteArray& getMainGeometry(void) const { return m_mainGeometry; }
	void setMainGeometry(const QByteArray& geometry);

	const QString& getDefaultFilterScript(void) const { return m_defaultFilterScript; }

	enum { MAX_LOG_COUNTS_RANGE_MIN=10, MAX_LOG_COUNTS_RANGE_MAX=10000000 };
	int getMaxLogCounts(void) const { return m_maxLogCounts; }
	void setMaxLogCounts(int maxLogCounts);

	enum { MAX_ACTOR_LOG_COUNTS_RANGE_MIN = 5, MAX_ACTOR_LOG_COUNTS_RANGE_MAX = 100 };
	int getMaxActorLogCounts(void) const { return m_maxActorLogCounts; }
	void setMaxActorLogCounts(qint32 maxActorLogCounts);

public:
	typedef LogParser::DefineMap LogParserDefineMap;
	typedef LogParser::DefinePtr LogParserDefinePtr;


	const LogParserDefinePtr getLogParser(const QString& title) const;
	const QString& getLoaParserScript(void) const {
		return m_logParserDefineScript;
	}
	const QString& getDefaultLogParserScript(void) const {
		return m_defaultLogParserDefineScript;
	}
	void setLogParserScript(const QString& logParserScript);

private:
	void _resetToDefaultSetting(void);

private:
	bool m_bCapture;
	bool m_bAutoScroll;
	bool m_bShowGrid;
	QString m_filterScript;
	QString m_defaultFilterScript;
	QByteArray m_mainGeometry;
	qint32 m_maxLogCounts;

	QString m_logParserDefineScript;
	QString m_defaultLogParserDefineScript;
	LogParserDefineMap m_logParserDefineMap;
	LogParserDefinePtr m_emptyLogParser;

	qint32 m_maxActorLogCounts;

public:
	Config();
	Config(const Config& other);
	~Config();
};
