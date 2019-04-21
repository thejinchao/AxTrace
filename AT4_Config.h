/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

class Config
{
public:
	void loadSetting(void);
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

	int getMaxLogCounts(void) const { return m_maxLogCounts; }
	void setMaxLogCounts(int maxLogCounts);

private:
	void _resetToDefaultSetting(void);

private:
	bool m_bCapture;
	bool m_bAutoScroll;
	bool m_bShowGrid;
	QString m_filterScript;
	QString m_defaultFilterScript;
	QByteArray m_mainGeometry;
	int m_maxLogCounts;

public:
	Config();
	Config(const Config& other);
	~Config();
};
