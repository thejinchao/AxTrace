/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#include "stdafx.h"
#include "AT4_LuaVirtualMachine.h"
#include "AT4_ChildWnd_Value.h"
#include "AT4_Message.h"
#include "AT4_ChildInterface.h"
#include "AT4_System.h"
#include "AT4_MainWindow.h"

//--------------------------------------------------------------------------------------------
ValueDataModel::ValueDataModel(QObject* parent)
	: QAbstractItemModel(parent)
	, m_view((QTreeView*)parent)
{

}

//--------------------------------------------------------------------------------------------
ValueDataModel::~ValueDataModel()
{

}

//--------------------------------------------------------------------------------------------
void ValueDataModel::insertValue(const ValueMessage* valueMessage, const ValueFilterResult& filterResult)
{
	const QString& valueName = valueMessage->getName();

	QString valueData;
	valueMessage->getValueAsString(valueData);

	const MessageTime updateTime = valueMessage->getTime();
	const QTime t = QDateTime::fromMSecsSinceEpoch(updateTime.epochTime).time();
	const QString updateTimeStr = tr("%1:%2 %3.%4")
		.arg(t.hour(), 2, 10, QLatin1Char('0'))
		.arg(t.minute(), 2, 10, QLatin1Char('0'))
		.arg(t.second(), 2, 10, QLatin1Char('0'))
		.arg(t.msec(), 3, 10, QLatin1Char('0'));

	const QColor backColor = LuaVirtualMachine::toQColor(filterResult.backColor);
	const QColor frontColor = LuaVirtualMachine::toQColor(filterResult.fontColor);

	const auto it = m_valueHashMap.constFind(valueName);
	if (it == m_valueHashMap.cend())
	{
		const auto valueIndex = m_valueVector.size();
		const int sequenceNumber = static_cast<int>(valueIndex);

		if (m_sortColumn == COLUMN_NONE)
		{
			const int newRow = m_sortedValues.size();
			beginInsertRows({}, newRow, newRow);
			m_valueVector.append({ sequenceNumber, updateTime, updateTimeStr, valueName, valueData, backColor, frontColor });
			m_valueHashMap.insert(valueName, valueIndex);
			m_sortedValues.append(valueIndex);
			endInsertRows();
		}
		else
		{
			m_valueVector.append({ sequenceNumber, updateTime, updateTimeStr, valueName, valueData, backColor, frontColor });
			m_valueHashMap.insert(valueName, valueIndex);
			const auto insertionPoint = std::upper_bound(
				m_sortedValues.cbegin(), m_sortedValues.cend(), valueIndex,
				[this](ValueVector::size_type newIndex, ValueVector::size_type existingIndex)
				{
					return lessThan(newIndex, existingIndex);
				}
			);
			const int newRow = static_cast<int>(std::distance(m_sortedValues.cbegin(), insertionPoint));
			beginInsertRows({}, newRow, newRow);
			m_sortedValues.insert(newRow, valueIndex);
			endInsertRows();
		}
	}
	else
	{
		const auto valueIndex = it.value();
		Value& value = m_valueVector[valueIndex];
		value.updateTime = updateTime;
		value.updateTimeStr = updateTimeStr;
		value.valueData = valueData;
		value.backColor = backColor;
		value.frontColor = frontColor;

		if (m_sortColumn != COLUMN_UPDATE_TIME)
		{
			const auto row = m_sortedValues.indexOf(valueIndex);
			emit dataChanged(index(row, COLUMN_UPDATE_TIME), index(row, COLUMN_VALUE_DATA), { Qt::DisplayRole });
		}
		else
		{
			const int oldRow = 	static_cast<int>(m_sortedValues.indexOf(valueIndex));
			const int newRow = m_sortOrder == Qt::AscendingOrder ? static_cast<int>(m_sortedValues.size() - 1) : 0;

			if (oldRow != newRow) 
			{
				const int destinationChild = m_sortOrder == Qt::AscendingOrder ? static_cast<int>(m_sortedValues.size()) : 0;
				beginMoveRows({}, oldRow, oldRow, {}, destinationChild);
				m_sortedValues.removeAt(oldRow);
				m_sortedValues.insert(newRow, valueIndex);
				endMoveRows();
			}

			emit dataChanged(index(newRow, COLUMN_UPDATE_TIME), index(newRow, COLUMN_VALUE_DATA), { Qt::DisplayRole });
		}

		int lineCounts = valueData.count('\n');
		bool needLayout = false;
		if (value.lineCounts != lineCounts)
		{
			value.lineCounts = valueData.count('\n');
			needLayout = true;
		}
		if (needLayout) m_view->doItemsLayout();
	}
}

//--------------------------------------------------------------------------------------------
void ValueDataModel::clearAllValue(void)
{
	beginRemoveRows(QModelIndex(), 0, rowCount()-1);
	m_valueVector.clear();
	m_valueHashMap.clear();
	endRemoveRows();
}

//--------------------------------------------------------------------------------------------
QVariant ValueDataModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() >= m_sortedValues.size() || index.column() >= COLUMN_COUNTS)
		return QVariant();

	const Value& value = m_valueVector[m_sortedValues[index.row()]];

	switch (role)
	{
	case Qt::BackgroundRole: return QBrush(value.backColor);
	case Qt::ForegroundRole: return QBrush(value.frontColor);
	case Qt::TextAlignmentRole: return QVariant(int(Qt::AlignLeft | Qt::AlignTop));
	case Qt::DisplayRole: return data(index.row(), index.column());
	default: return QVariant();
	}
}

//--------------------------------------------------------------------------------------------
QString ValueDataModel::data(int row, int column) const
{
	if (row >= m_sortedValues.size() || column >= COLUMN_COUNTS)
		return QString();

	const Value& value = m_valueVector[m_sortedValues[row]];
	switch (column)
	{
	case COLUMN_INDEX: return QString::number(value.index);
	case COLUMN_UPDATE_TIME: return value.updateTimeStr;
	case COLUMN_VALUE_NAME: return value.valueName;
	case COLUMN_VALUE_DATA: return value.valueData;
		default: return QString();
	}
}

//--------------------------------------------------------------------------------------------
QVariant ValueDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
		case COLUMN_INDEX: return QVariant(tr("#"));
		case COLUMN_UPDATE_TIME: return QVariant(tr("Time"));
		case COLUMN_VALUE_NAME: return QVariant(tr("Name"));
		case COLUMN_VALUE_DATA: return QVariant(tr("Value"));
		default: return QVariant();
		}
	}

	return QVariant();
}

//--------------------------------------------------------------------------------------------
QModelIndex ValueDataModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	if (!parent.isValid())
		return createIndex(row, column);
	else
		return QModelIndex();
}

//--------------------------------------------------------------------------------------------
void ValueDataModel::sort(int column, Qt::SortOrder order)
{
	if (column < COLUMN_INDEX || column > COLUMN_VALUE_NAME) return;

	emit layoutAboutToBeChanged({}, VerticalSortHint);

	const QModelIndexList oldPersistentIndexes = persistentIndexList();
	ValueIndexVector persistentValueIndexes;
	persistentValueIndexes.reserve(oldPersistentIndexes.size());
	for (const QModelIndex& persistentIndex : oldPersistentIndexes) 
	{
		persistentValueIndexes.append(m_sortedValues.at(persistentIndex.row()));
	}

	m_sortColumn = static_cast<ValueColumn>(column);
	m_sortOrder = order;
	std::stable_sort(
		m_sortedValues.begin(), m_sortedValues.end(),
		[this](QVector<Value>::size_type leftIndex, QVector<Value>::size_type rightIndex)
		{
			return lessThan(leftIndex, rightIndex);
		}
	);

	QModelIndexList newPersistentIndexes;
	newPersistentIndexes.reserve(oldPersistentIndexes.size());
	for (qsizetype i = 0; i < oldPersistentIndexes.size(); ++i) 
	{
		const int newRow = m_sortedValues.indexOf(persistentValueIndexes.at(i));
		newPersistentIndexes.append(
			index(newRow, oldPersistentIndexes.at(i).column()));
	}
	changePersistentIndexList(oldPersistentIndexes, newPersistentIndexes);
	emit layoutChanged({}, VerticalSortHint);
}

//--------------------------------------------------------------------------------------------
bool ValueDataModel::lessThan(ValueVector::size_type leftIndex, ValueVector::size_type rightIndex) const
{
	const Value& left = m_valueVector.at(leftIndex);
	const Value& right = m_valueVector.at(rightIndex);

	if(m_sortColumn == COLUMN_UPDATE_TIME)
	{
		if (left.updateTime.epochTime != right.updateTime.epochTime)
		{
			return m_sortOrder == Qt::AscendingOrder ? left.updateTime.epochTime < right.updateTime.epochTime : left.updateTime.epochTime > right.updateTime.epochTime;
		}
	}
	else if(m_sortColumn == COLUMN_VALUE_NAME)
	{
		const int comparison = QString::localeAwareCompare(left.valueName, right.valueName);
		if (comparison != 0)
		{
			return m_sortOrder == Qt::AscendingOrder ? comparison < 0 : comparison > 0;
		}
	}

	return m_sortOrder == Qt::AscendingOrder ? left.index < right.index : left.index > right.index;
}

//--------------------------------------------------------------------------------------------
class ValueChildInterface : public IChild
{
public:
	virtual Type getType(void) const { return CT_VALUE; }

	virtual bool copyAble(void) const {
		return !(m_proxy->selectionModel()->selectedRows().empty());
	}

	virtual bool isPause(void) const {
		return m_proxy->isPause();
	}

	virtual void switchPause(void) {
		m_proxy->switchPause();
	}

	virtual void onCopy(void) const
	{
		ValueDataModel* model = (ValueDataModel*)(m_proxy->model());

		QModelIndexList rows = m_proxy->selectionModel()->selectedRows();
		//sort by id
		std::sort(rows.begin(), rows.end(), [model](const QModelIndex &s1, const QModelIndex &s2) {
			return s1.row() < s2.row();
		});

		QString lines;
		foreach(auto row, rows)
		{
			QString line = QString("%1\t%2\t%3\t%4\n").arg(
				model->data(row.row(), 0),
				model->data(row.row(), 1),
				model->data(row.row(), 2),
				model->data(row.row(), 3));

			lines += line;
		}

		QApplication::clipboard()->setText(lines);
	}

	virtual QString getTitle(void) const 
	{
		return m_proxy->windowTitle();
	}

	virtual void clean(void)
	{
		ValueDataModel* model = (ValueDataModel*)(m_proxy->model());
		model->clearAllValue();
	}
	
	virtual void saveAs(void)
	{
		QString fileName = QFileDialog::getSaveFileName(nullptr, QString("Save As..."), QString("axtrace.log"), QString("Log file (*.log *.txt)"));
		if (fileName.isEmpty()) return;

		ValueDataModel* model = (ValueDataModel*)(m_proxy->model());

		QFile file(fileName);
		if (file.open(QFile::WriteOnly))
		{
			QTextStream stream(&file);
			for (int rowIndex = 0; rowIndex < model->rowCount(); rowIndex++)
			{
				QString line = QString("%1\t%2\t%3\t%4\n").arg(
					model->data(rowIndex, 0),
					model->data(rowIndex, 1),
					model->data(rowIndex, 2),
					model->data(rowIndex, 3));
				stream << line;
			}
			file.close();
		}
	}

	virtual void update(void)
	{
		m_proxy->update();
	}
private:
	ValueChild* m_proxy;

public:
	ValueChildInterface(ValueChild* proxy) : m_proxy(proxy) { }
	~ValueChildInterface() {}
};

//--------------------------------------------------------------------------------------------
ValueChild::ValueChild(const QString& title)
	: m_pause(false)
{
	setAttribute(Qt::WA_DeleteOnClose);
	
	m_title = title;
	QString windowTitle = tr("Value:%1").arg(title);
	setWindowTitle(windowTitle);

	QVariant v;
	v.setValue(ChildVariant(new ValueChildInterface(this)));
	this->setProperty(ValueChildInterface::PropertyName, v);
}

//--------------------------------------------------------------------------------------------
ValueChild::~ValueChild()
{
	QVariant v = this->property(ValueChildInterface::PropertyName);
	ValueChildInterface* i = (ValueChildInterface*)(v.value<ChildVariant>().child);
	delete i;

	this->setProperty(ValueChildInterface::PropertyName, QVariant());
}

//--------------------------------------------------------------------------------------------
void ValueChild::init(void)
{
	this->setModel(new ValueDataModel(this));
	this->header()->resizeSection(ValueDataModel::COLUMN_INDEX, 40);
	this->header()->resizeSection(ValueDataModel::COLUMN_UPDATE_TIME, 120);
	this->header()->resizeSection(ValueDataModel::COLUMN_VALUE_NAME, 200);
	this->setSortingEnabled(false);
	this->setRootIsDecorated(false);
	this->setAllColumnsShowFocus(true);
	this->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->setEditTriggers(QAbstractItemView::NoEditTriggers);
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->setUniformRowHeights(false);
	this->setAlternatingRowColors(false);
	this->header()->setSectionsClickable(true);
	this->header()->setStretchLastSection(true);
	this->header()->setSortIndicatorShown(false);

	connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, []() {
		System::getSingleton()->getMainWindow()->notifySelectionChanged();
	});

	connect(this->header(), &QHeaderView::sectionClicked, this, &ValueChild::sortByHeader);
}

//--------------------------------------------------------------------------------------------
void ValueChild::insertValue(const ValueMessage* valueMessage, const ValueFilterResult& filterResult)
{
	if (m_pause) return;

	ValueDataModel* model = (ValueDataModel*)(this->model());

	model->insertValue(valueMessage, filterResult);
}

//--------------------------------------------------------------------------------------------
void ValueChild::closeEvent(QCloseEvent *event)
{
	System::getSingleton()->getMainWindow()->notifySubWindowClose(IChild::CT_VALUE, m_title);
	event->accept();
}

//--------------------------------------------------------------------------------------------
void ValueChild::clearAllValue(void)
{
	ValueDataModel* model = (ValueDataModel*)(this->model());

	model->clearAllValue();
}

//--------------------------------------------------------------------------------------------
void ValueChild::switchPause(void)
{
	m_pause = !m_pause;
}

//--------------------------------------------------------------------------------------------
void ValueChild::sortByHeader(int column)
{
	ValueDataModel* model = (ValueDataModel*)(this->model());
	ValueDataModel::ValueColumn sortColumn = model->getSortColumn();
	Qt::SortOrder sortOrder = model->getSortOrder();

	if (column < 0 || column >= ValueDataModel::COLUMN_VALUE_DATA) 
	{
		if (sortColumn >= ValueDataModel::COLUMN_INDEX)
		{
			header()->setSortIndicator(sortColumn, sortOrder);
		}
		else 
		{
			header()->setSortIndicatorShown(false);
		}
		return;
	}

	if (sortColumn == column) 
	{
		sortOrder = (sortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
	}
	else 
	{
		sortColumn = (ValueDataModel::ValueColumn)column;
		sortOrder = Qt::AscendingOrder;
	}

	header()->setSortIndicatorShown(true);
	header()->setSortIndicator(sortColumn, sortOrder);
	model->sort(sortColumn, sortOrder);
}

