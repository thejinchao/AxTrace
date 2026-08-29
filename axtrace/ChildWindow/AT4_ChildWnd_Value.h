/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#pragma once

#include <QTreeView>
#include "Data/AT4_ValueData.h"

class ValueMessage;

class ValueDataModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit ValueDataModel(QObject *parent = 0);
	~ValueDataModel();

	void insertValue(const ValueMessage* valueMessage, const ValueFilterResult& filterResult);
	void clearAllValue(void);

	QVariant data(const QModelIndex &index, int role) const override;
	QString data(int row, int column) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override {
		return QModelIndex();
	}
	int rowCount(const QModelIndex &parent = QModelIndex()) const override {
		return m_valueVector.size();
	}
	int columnCount(const QModelIndex &parent = QModelIndex()) const override {
		return COLUMN_COUNTS;
	}

public:
	enum { 
		COLUMN_INDEX = 0,
		COLUMN_UPDATE_TIME = 1,
		COLUMN_VALUE_NAME = 2,
		COLUMN_VALUE_DATA = 3,

		COLUMN_COUNTS = 4
	};

private:
	ValueVector m_valueVector;
	ValueHashMap m_valueHashMap;

	QTreeView* m_view;
};


class ValueChild : public QTreeView
{
	Q_OBJECT

public:
	void init(void);
	void insertValue(const ValueMessage* valueMessage, const ValueFilterResult& filterResult);
	void clearAllValue(void);
	bool isPause(void) const { return m_pause; }
	void switchPause(void);

private:
	QString m_title;
	bool m_pause;

protected:
	void closeEvent(QCloseEvent *event) override;

public:
	ValueChild(const QString& title);
	virtual ~ValueChild();
};
