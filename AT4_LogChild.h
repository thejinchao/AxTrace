/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include <QTreeView>
#include <QItemDelegate>
#include <QQueue>
#include "AT4_Filter.h"
#include "AT4_LogData.h"
#include "AT4_LogColumn.h"

class LogMessage;

class LogDataModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit LogDataModel(QObject *parent = 0);
	~LogDataModel();

	void initDefaultColumn(void);
	void insertLog(const LogMessage* logMessage, const Filter::ListResult& filterResult);
	void clearAllLog(void);
	void autoCheckOverflow(void);

	QVariant data(const QModelIndex &index, int role) const override;
	QString data(int row, int column) const;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override {
		return QModelIndex();
	}
	int rowCount(const QModelIndex &parent = QModelIndex()) const override {
		return m_logVector.size();
	}

	LogColumnVector& getColumns(void) {
		return m_logColumnVector;
	}
	int columnCount(const QModelIndex &parent = QModelIndex()) const override {
		return m_logColumnVector.size();
	}

private:
	enum { DEFAULT_MAX_OVERFLOW_COUNTS=10 };

	LogColumnVector m_logColumnVector;
	LogDataVector m_logVector;
	quint32 m_currentIndex;
	qint32 m_maxLogCounts;
	qint32 m_maxOverflowCounts;
};

class LogChild : public QTreeView
{
    Q_OBJECT

public:
	void init(void);
	void insertLog(const LogMessage* logMessage, const Filter::ListResult& filterResult);

protected:
    void closeEvent(QCloseEvent *event) override;
	void timerEvent(QTimerEvent *event) override;

private:
	QString m_title;
	bool m_bNeedScrollDown;

public:
	LogChild(const QString& title);
	virtual ~LogChild();
};
