/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
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

	bool getShowTail(void) const { return m_bShowTail; }
	void setShowTail(bool s) { m_bShowTail = s; }

	const QString& getFilterScript(void) const { return m_filterScript; }
	void setFilterScript(const QString& script);
	const QString& getDefaultFilterScript(void) const { return m_defaultFilterScript; }

	const QByteArray& getMainGeometry(void) const { return m_mainGeometry; }
	void setMainGeometry(const QByteArray& geometry);

	enum {
		MAX_LOG_COUNTS_DEFAULT = 10000, MAX_LOG_COUNTS_RANGE_MIN = 10, MAX_LOG_COUNTS_RANGE_MAX = 10000000
	};
	int getMaxLogCounts(void) const { return m_maxLogCounts; }
	void setMaxLogCounts(int maxLogCounts);

	enum { MAX_ACTOR_LOG_COUNTS_DEFAULT = 30, MAX_ACTOR_LOG_COUNTS_RANGE_MIN = 5, MAX_ACTOR_LOG_COUNTS_RANGE_MAX = 100 };
	int getMaxActorLogCounts(void) const { return m_maxActorLogCounts; }
	void setMaxActorLogCounts(qint32 maxActorLogCounts);

	enum { MAX_ACTOR_TAIL_COUNTS_DEFAULT = 200, MAX_ACTOR_TAIL_COUNTS_RANGE_MIN = 10, MAX_ACTOR_TAIL_COUNTS_RANGE_MAX = 2000 };
	int getMaxActorTailCounts(void) const { return m_maxActorTailCounts; }
	void setMaxActorTailCounts(qint32 maxActorTailCounts);

	enum { LISTEN_PORT_DEFAULT = 1978, LISTEN_PORT_MIN = 1025, LISTEN_PORT_MAX = 65535};
	int getListenPort(void) const { return m_listenPort; }
	void setListenPort(qint32 listenPort);

public:
	const LogParserPtr getLogParser(const QString& title) const;
	const QString& getLoaParserScript(void) const {
		return m_logParserDefineScript;
	}
	void setLogParserScript(const QString& logParserScript);
	const QString& getDefaultLogParserScript(void) const {
		return m_defaultLogParserDefineScript;
	}

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
	bool m_bShowTail;
	qint32 m_maxActorTailCounts;

	QString m_logParserDefineScript;
	QString m_defaultLogParserDefineScript;

	LogParserVector m_logParserVector;
	LogParserPtr m_defaultLogParser;

	qint32 m_maxActorLogCounts;

	qint32 m_listenPort;
public:
	Config();
	Config(const Config& other);
	~Config();
};
