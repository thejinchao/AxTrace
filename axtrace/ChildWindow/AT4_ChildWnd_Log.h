/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#pragma once

#include <QTreeView>
#include <QItemDelegate>
#include <QQueue>
#include "AT4_Filter.h"
#include "AT4_LogData.h"
#include "AT4_LogColumn.h"
#include "AT4_Config.h"

class LogMessage;

class LogDataModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit LogDataModel(LogParserPtr logParser, QObject *parent = 0);
	~LogDataModel();

	void insertLog(const LogMessage* logMessage, const Filter::ListResult& filterResult);
	void clearAllLog(void);
	void autoCheckOverflow(void);
	void switchColumn(qint32 index);

	QVariant data(const QModelIndex &index, int role) const override;
	QString data(int row, int column) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override {
		return QModelIndex();
	}
	int rowCount(const QModelIndex &parent = QModelIndex()) const override {
		return m_logVector.size();
	}

	const LogColumnGroup& getColumns(void) const {
		return m_logColumnGroup;
	}
	LogColumnGroup& getColumns(void) {
		return m_logColumnGroup;
	}

	int columnCount(const QModelIndex &parent = QModelIndex()) const override {
		return m_logColumnGroup.getActiveCounts();
	}

private:
	enum { LOG_INDEX_START_FROM=1, DEFAULT_MAX_OVERFLOW_COUNTS=10 };

	LogColumnGroup m_logColumnGroup;
	LogDataVector m_logVector;
	quint32 m_currentIndex;
	qint32 m_maxOverflowCounts;
	LogParserPtr m_logParser;
};

class LogChild : public QTreeView
{
    Q_OBJECT

public:
	void init(void);
	void insertLog(const LogMessage* logMessage, const Filter::ListResult& filterResult);
	bool isPause(void) const { return m_pause; }
	void switchPause(void);

protected:
	bool eventFilter(QObject *target, QEvent *event);
	void closeEvent(QCloseEvent *event) override;
	void timerEvent(QTimerEvent *event) override;
	
	void onHeadContextMenu(const QPoint & pos);

private:
	QString m_title;
	bool m_pause;
	bool m_bNeedScrollDown;

public:
	LogChild(const QString& title);
	virtual ~LogChild();
};
