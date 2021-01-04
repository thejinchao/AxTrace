/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"

#include "AT4_ValueChild.h"
#include "AT4_Message.h"
#include "AT4_ChildInterface.h"
#include "AT4_System.h"
#include "AT4_MainWindow.h"

//--------------------------------------------------------------------------------------------
ValueDataModel::ValueDataModel(QObject *parent)
	: QAbstractItemModel(parent)
{

}

//--------------------------------------------------------------------------------------------
ValueDataModel::~ValueDataModel()
{

}

//--------------------------------------------------------------------------------------------
void ValueDataModel::insertValue(const ValueMessage* valueMessage, const Filter::ListResult& filterResult)
{
	const QString& name = valueMessage->getName();
	int idx;

	auto it = m_valueHashMap.find(name);
	if (it == m_valueHashMap.end())
	{
		idx = m_valueVector.size();

		Value value;
		value.valueName = name;

		m_valueVector.push_back(value);
		m_valueHashMap.insert(name, idx);
	}
	else
	{
		idx = it.value();
	}

	beginInsertRows(QModelIndex(), idx, idx);

	Value& value = m_valueVector[idx];

	const MessageTime& t = valueMessage->getTime();
	value.updateTime = tr("%1:%2 %3.%4")
		.arg(t.hour, 2, 10, QLatin1Char('0'))
		.arg(t.minute, 2, 10, QLatin1Char('0'))
		.arg(t.second, 2, 10, QLatin1Char('0'))
		.arg(t.milliseconds, 3, 10, QLatin1Char('0'));

	QString valueData;
	valueMessage->getValueAsString(valueData);
	value.valueData = valueData;

	value.backColor = Filter::toQColor(filterResult.backColor);
	value.frontColor = Filter::toQColor(filterResult.fontColor);
	endInsertRows();

	dataChanged(index(idx, 0), index(idx, COLUMN_COUNTS));
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
	if (!index.isValid() || index.row() >= m_valueVector.size() || index.column() >= COLUMN_COUNTS)
		return QVariant();

	const Value& value = m_valueVector[index.row()];

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
	if (row >= m_valueVector.size() || column >= COLUMN_COUNTS)
		return QString();

	const Value& value = m_valueVector[row];
	switch (column)
	{
	case 0: return value.updateTime;
	case 1: return value.valueName;
	case 2: return value.valueData;
	default: return QString();
	}
}

//--------------------------------------------------------------------------------------------
QVariant ValueDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == 0) return QVariant(tr("Time"));
		else if (section == 1) return QVariant(tr("Name"));
		else return QVariant(tr("Value"));
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
			QString line = QString("%1\t%2\t%3\n").arg(
				model->data(row.row(), 0),
				model->data(row.row(), 1),
				model->data(row.row(), 2));

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
				QString line = QString("%1\t%2\t%3\n").arg(
					model->data(rowIndex, 0),
					model->data(rowIndex, 1),
					model->data(rowIndex, 2));
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
	this->setModel(new ValueDataModel());
	this->header()->resizeSection(0, 120);
	this->header()->resizeSection(1, 200);
	this->setSortingEnabled(false);
	this->setRootIsDecorated(false);
	this->setSelectionMode(MultiSelection);

	connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, []() {
		System::getSingleton()->getMainWindow()->notifySelectionChanged();
	});
}

//--------------------------------------------------------------------------------------------
void ValueChild::insertValue(const ValueMessage* valueMessage, const Filter::ListResult& filterResult)
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
