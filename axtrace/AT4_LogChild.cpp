﻿/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"

#include "AT4_LogChild.h"
#include "AT4_Message.h"
#include "AT4_ChildInterface.h"
#include "AT4_System.h"
#include "AT4_Config.h"
#include "AT4_MainWindow.h"

//--------------------------------------------------------------------------------------------
LogDataModel::LogDataModel(const Config::LogParserDefinePtr logParserDefine, QObject *parent)
	: QAbstractItemModel(parent)
	, m_currentIndex(LOG_INDEX_START_FROM)
	, m_maxOverflowCounts(DEFAULT_MAX_OVERFLOW_COUNTS)
	, m_logParser(logParserDefine)
{
	m_logColumnGroup.initDefaulGroup(m_logParser);
}

//--------------------------------------------------------------------------------------------
LogDataModel::~LogDataModel()
{

}

//--------------------------------------------------------------------------------------------
void LogDataModel::insertLog(const LogMessage* logMessage, const Filter::ListResult& filterResult)
{
	LogData logData;
	logData.logIndex = m_currentIndex++;
	logData.logTime = logMessage->getTime();
	logData.session = logMessage->getSession();
	logData.logType = logMessage->getLogType();
	logData.backColor = Filter::toQColor(filterResult.backColor);
	logData.frontColor = Filter::toQColor(filterResult.fontColor);
	logData.logContent = m_logParser.parserLog(logMessage->getLog());

	beginInsertRows(QModelIndex(), rowCount(), rowCount()+1);

	m_logVector.push_back(logData);

	endInsertRows();
}

//--------------------------------------------------------------------------------------------
void LogDataModel::clearAllLog(void)
{
	beginRemoveRows(QModelIndex(), 0, rowCount()-1);
	m_logVector.clear();
	endRemoveRows();
}

//--------------------------------------------------------------------------------------------
void LogDataModel::autoCheckOverflow(void)
{
	qint32 currentCounts = m_logVector.size();
	qint32 maxLogCounts = System::getSingleton()->getConfig()->getMaxLogCounts();

	if (currentCounts > maxLogCounts + m_maxOverflowCounts) {
		int needRemove = currentCounts - maxLogCounts;

		beginRemoveRows(QModelIndex(), 0, needRemove-1);
		m_logVector.erase(m_logVector.begin(), m_logVector.begin() + needRemove);
		endRemoveRows();
	}
}

//--------------------------------------------------------------------------------------------
QVariant LogDataModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() >= m_logVector.size() || index.column() >= this->columnCount())
		return QVariant();

	const LogData& logData = m_logVector[index.row()];

	switch (role)
	{
	case Qt::BackgroundRole: return QBrush(logData.backColor);
	case Qt::ForegroundRole: return QBrush(logData.frontColor);
	case Qt::TextAlignmentRole: return QVariant(int(Qt::AlignLeft | Qt::AlignTop));
	case Qt::DisplayRole: return data(index.row(), index.column());
	default: return QVariant();
	}
}

//--------------------------------------------------------------------------------------------
QString LogDataModel::data(int row, int column) const
{
	if (row >= m_logVector.size() || column >= this->columnCount())
		return QString();

	const LogData& logData = m_logVector[row];
	if (column >= 0 && column < m_logColumnGroup.getActiveCounts())
	{
		return m_logColumnGroup.getActiveColumn(column)->getString(logData);
	}
	return QString();
}

//--------------------------------------------------------------------------------------------
Qt::ItemFlags LogDataModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	return QAbstractItemModel::flags(index);
}

//--------------------------------------------------------------------------------------------
QVariant LogDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section >= 0 && section < m_logColumnGroup.getActiveCounts())
		{
			return m_logColumnGroup.getActiveColumn(section)->getTitle();
		}
		return QString();
	}

	return QVariant();
}

//--------------------------------------------------------------------------------------------
QModelIndex LogDataModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	if (!parent.isValid())
		return createIndex(row, column);
	else
		return QModelIndex();
}

//--------------------------------------------------------------------------------------------
void LogDataModel::switchColumn(qint32 index)
{
	Q_ASSERT(index>=0 && index< m_logColumnGroup.getCounts()-1);

	const LogColumn* column = m_logColumnGroup.getColumn(index);
	if (column->isActive())
	{
		this->beginRemoveColumns(QModelIndex(), column->getActiveIndex(), column->getActiveIndex());
		m_logColumnGroup.activeColumn(index, !(column->isActive()));
		this->endRemoveColumns();
	}
	else
	{
		this->beginInsertColumns(QModelIndex(), column->getActiveIndex(), column->getActiveIndex());
		m_logColumnGroup.activeColumn(index, !(column->isActive()));
		this->endInsertColumns();
	}
}

//--------------------------------------------------------------------------------------------
class LogChildInterface : public IChild
{
public:
	virtual Type getType(void) const { return CT_LOG; }

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
		LogDataModel* model = (LogDataModel*)(m_proxy->model());
		const LogColumnGroup& columnGroup = model->getColumns();

		QModelIndexList rows = m_proxy->selectionModel()->selectedRows();
	
		//sort by id
		qSort(rows.begin(), rows.end(), [model](const QModelIndex &s1, const QModelIndex &s2){ 
			return s1.row() < s2.row();
		});

		QString lines;
		foreach(auto row, rows)
		{
			int rowIndex = row.row();

			QString line;
			model->getColumns().walk(true, [&](const LogColumn* column) {
				line += model->data(rowIndex, column->getActiveIndex());

				if (column->getActiveIndex() == columnGroup.getActiveCounts() - 1) line += "\n";
				else line += "\t";
			});

			lines += line;
		}

		QApplication::clipboard()->setText(lines);
	}

	virtual QString getTitle(void) const {
		return m_proxy->windowTitle();
	}

	virtual void clean(void)
	{
		LogDataModel* model = (LogDataModel*)(m_proxy->model());

		model->clearAllLog();
		update();
	}

	virtual void saveAs(void)
	{
		QString fileName = QFileDialog::getSaveFileName(nullptr, QString("Save As..."), QString("axtrace.log"), QString("Log file (*.log *.txt)"));
		if (fileName.isEmpty()) return;

		LogDataModel* model = (LogDataModel*)(m_proxy->model());
		const LogColumnGroup& columnGroup = model->getColumns();

		QFile file(fileName);
		if (file.open(QFile::WriteOnly))
		{
			QTextStream stream(&file);
			for (int rowIndex = 0; rowIndex < model->rowCount(); rowIndex++)
			{
				QString line;
				model->getColumns().walk(true, [&](const LogColumn* column) {
					line += model->data(rowIndex, column->getActiveIndex());

					if (column->getActiveIndex() == columnGroup.getActiveCounts() - 1) line += "\n";
					else line += "\t";
				});

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
	LogChild* m_proxy;

public:
	LogChildInterface(LogChild* proxy) : m_proxy(proxy) { }
	~LogChildInterface() {}
};

//--------------------------------------------------------------------------------------------
LogChild::LogChild(const QString& title)
	: m_pause(false)
{
    setAttribute(Qt::WA_DeleteOnClose);

	m_title = title;
	QString windowTitle = tr("Log:%1").arg(title);
	setWindowTitle(windowTitle);

	this->setUserData(0, new LogChildInterface(this));
	
	m_bNeedScrollDown = false;
}

//--------------------------------------------------------------------------------------------
LogChild::~LogChild()
{
	LogChildInterface* i = (LogChildInterface*)(this->userData(0));
	delete i;

	this->setUserData(0, nullptr);
}

//--------------------------------------------------------------------------------------------
void LogChild::init(void)
{
	Config* config = System::getSingleton()->getConfig();
	LogDataModel* model = new LogDataModel(config->getLogParser(m_title));
	this->setModel(model);

	QHeaderView* header = this->header();

	header->installEventFilter(this);
	header->setSectionsMovable(false);
	header->setSectionResizeMode(QHeaderView::Interactive);

	model->getColumns().walk(true, [&, header](const LogColumn* column) {
		if (column->isActive() && column->getWidth() > 0) {
			header->resizeSection(column->getActiveIndex(), column->getWidth());
		}
	});

	this->setSortingEnabled(false);
	this->setRootIsDecorated(false);
	this->setSelectionMode(MultiSelection);
	
	connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, []() {
		System::getSingleton()->getMainWindow()->notifySelectionChanged();
	});

	this->startTimer(100);
}

//--------------------------------------------------------------------------------------------
void LogChild::closeEvent(QCloseEvent *event)
{
	System::getSingleton()->getMainWindow()->notifySubWindowClose(IChild::CT_LOG, m_title);
	event->accept();
}

//--------------------------------------------------------------------------------------------
void LogChild::switchPause(void)
{
	m_pause = !m_pause;
}

//--------------------------------------------------------------------------------------------
void LogChild::timerEvent(QTimerEvent *event)
{
	if (m_bNeedScrollDown && System::getSingleton()->getConfig()->getAutoScroll()) {

		this->scrollToBottom();
		m_bNeedScrollDown = false;
	}

	LogDataModel* model = (LogDataModel*)(this->model());
	if(model)
		model->autoCheckOverflow();
}

//--------------------------------------------------------------------------------------------
void LogChild::insertLog(const LogMessage* logMessage, const Filter::ListResult& filterResult)
{
	if (m_pause) return;

	LogDataModel* model = (LogDataModel*)(this->model());

	model->insertLog(logMessage, filterResult);

	m_bNeedScrollDown = true;
	update();
}

//--------------------------------------------------------------------------------------------
bool LogChild::eventFilter(QObject *target, QEvent *event)
{
	if (target == this->header())
	{
		if (event->type() == QEvent::ContextMenu)
		{
			//Create context menu here
			QContextMenuEvent* menuEvent = dynamic_cast<QContextMenuEvent*>(event);
			onHeadContextMenu(menuEvent->globalPos());
			return true;
		}
	}

	return false;

}

//--------------------------------------------------------------------------------------------
void LogChild::onHeadContextMenu(const QPoint & pos)
{
	LogDataModel* model = (LogDataModel*)(this->model());
	LogColumnGroup& columnGroup = model->getColumns();

	QMenu headContextMenu;
	columnGroup.walk(false, [&](const LogColumn* column) {
		QAction* action = headContextMenu.addAction(column->getTitle());

		action->setProperty("context", QVariant(column->getIndex()));
		action->setCheckable(true);
		action->setChecked(column->isActive());

		if (column->getIndex() == columnGroup.getCounts()-1)
		{
			action->setEnabled(false);
		}
	});

	QAction* selectedItem = headContextMenu.exec(pos);
	if (!selectedItem) return;

	qint32 index = selectedItem->property("context").toInt();
	Q_ASSERT(index >= 0 && index < columnGroup.getCounts());

	this->selectionModel()->clearSelection();
	model->switchColumn(index);
}
