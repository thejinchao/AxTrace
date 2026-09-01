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
	enum ValueColumn {
		COLUMN_NONE = -1,

		COLUMN_INDEX = 0,
		COLUMN_UPDATE_TIME = 1,
		COLUMN_VALUE_NAME = 2,
		COLUMN_VALUE_DATA = 3,

		COLUMN_COUNTS
	};

public:
	explicit ValueDataModel(QObject *parent = 0);
	~ValueDataModel();

	void insertValue(const ValueMessage* valueMessage, const ValueFilterResult& filterResult);
	void clearAllValue(void);
	ValueColumn getSortColumn(void) const { 
		return m_sortColumn; 
	}
	Qt::SortOrder getSortOrder(void) const {
		return m_sortOrder;
	}

	QVariant data(const QModelIndex &index, int role) const override;
	QString data(int row, int column) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override {
		return QModelIndex();
	}
	int rowCount(const QModelIndex &parent = QModelIndex()) const override {
		return parent.isValid() ? 0 : m_valueVector.size();
	}
	int columnCount(const QModelIndex &parent = QModelIndex()) const override {
		return parent.isValid() ? 0 : COLUMN_COUNTS;
	}
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
	bool lessThan(ValueVector::size_type leftIndex, ValueVector::size_type rightIndex) const;

private:
	ValueVector m_valueVector;
	ValueHashMap m_valueHashMap;
	ValueIndexVector m_sortedValues;

	ValueColumn m_sortColumn = COLUMN_NONE;
	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

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
	void sortByHeader(int column);

private:
	QString m_title;
	bool m_pause;

protected:
	void closeEvent(QCloseEvent *event) override;

public:
	ValueChild(const QString& title);
	virtual ~ValueChild();
};
