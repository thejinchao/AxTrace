/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
class QTreeView;
class QStandardItemModel;
QT_END_NAMESPACE

class SessionDialog : public QDialog
{
	Q_OBJECT

public:
	void updateSessionList();
	void updateSession(int32_t id);

private:
	QTreeView *m_sessionList;
	QStandardItemModel *m_sessionModel;

public:
	SessionDialog(QWidget *parent = 0);
	~SessionDialog();
};
