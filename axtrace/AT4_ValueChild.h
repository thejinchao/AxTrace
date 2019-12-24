/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include <QTreeView>
#include "AT4_Filter.h"

class ValueMessage;

class ValueDataModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit ValueDataModel(QObject *parent = 0);
	~ValueDataModel();

	void insertValue(const ValueMessage* valueMessage, const Filter::ListResult& filterResult);
	void clearAllValue(void);

	QVariant data(const QModelIndex &index, int role) const override;
	QString data(int row, int column) const;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
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

private:
	enum { COLUMN_COUNTS = 3 };

	struct Value
	{
		QString updateTime;
		QString valueName;
		QString valueData;
		QColor backColor;
		QColor frontColor;
	};

	typedef QVector<Value> ValueVector;
	typedef QHash<QString, ValueVector::size_type> ValueHashMap;

	ValueVector m_valueVector;
	ValueHashMap m_valueHashMap;
};


class ValueChild : public QTreeView
{
	Q_OBJECT

public:
	void init(void);
	void insertValue(const ValueMessage* valueMessage, const Filter::ListResult& filterResult);
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
