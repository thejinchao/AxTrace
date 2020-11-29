/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_SessionDialog.h"
#include "AT4_System.h"
#include "AT4_Session.h"

//--------------------------------------------------------------------------------------------
SessionDialog::SessionDialog(QWidget *parent)
	: QDialog(parent)
{
	//setup model
	m_sessionModel = new QStandardItemModel();

	m_sessionList = new QTreeView();
	m_sessionList->setModel(m_sessionModel);
	m_sessionList->setHeaderHidden(false);
	m_sessionList->setSortingEnabled(false);
	m_sessionList->setRootIsDecorated(false);

	m_sessionModel->setColumnCount(4);
	m_sessionModel->setHorizontalHeaderLabels({ "ID", "Name", "PID", "TID", "Address" });

	QHeaderView* header = m_sessionList->header();
	header->resizeSection(0, 50);
	header->resizeSection(1, 100);
	header->resizeSection(2, 80);
	header->resizeSection(3, 80);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_sessionList);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	QDialogButtonBox* closeBtn = new QDialogButtonBox(QDialogButtonBox::Close);

	connect(closeBtn, &QDialogButtonBox::rejected, this, &SessionDialog::reject);

	buttonLayout->addWidget(closeBtn);

	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);
	resize(480, 320);
	setWindowTitle(tr("Sessions"));

	//update session
	updateSessionList();
}

//--------------------------------------------------------------------------------------------
SessionDialog::~SessionDialog()
{
}

//--------------------------------------------------------------------------------------------
void SessionDialog::updateSessionList()
{
	SessionManager* sessionManager = System::getSingleton()->getSessionManager();

	//clear all sessions
	m_sessionModel->removeRows(0, m_sessionModel->rowCount());

	//fill session list
	sessionManager->walk([this](int32_t id, SessionPtr session) {

		int index = this->m_sessionModel->rowCount();
		session->setUserData(index);

		QStandardItem *item_id = new QStandardItem(QString("%0").arg(id));
		this->m_sessionModel->setItem(index, 0, item_id);

		QStandardItem *item_Name = new QStandardItem(QString("%0").arg(session->getSessionName()));
		this->m_sessionModel->setItem(index, 1, item_Name);

		QStandardItem *item_PID = new QStandardItem(QString("%0").arg(session->getProcessID()));
		this->m_sessionModel->setItem(index, 2, item_PID);

		QStandardItem *item_TID = new QStandardItem(QString("%0").arg(session->getThreadID()));
		this->m_sessionModel->setItem(index, 3, item_TID);

		QStandardItem *item_Address = new QStandardItem(QString("%0").arg(session->getPeerAddress()));
		this->m_sessionModel->setItem(index, 4, item_Address);
	});
}

//--------------------------------------------------------------------------------------------
void SessionDialog::updateSession(int32_t id)
{
	SessionManager* sessionManager = System::getSingleton()->getSessionManager();

	SessionPtr session = sessionManager->findSession(id);
	if (!session) return;

	int32_t index = session->getUserData();
	if (index >= 0 && index < m_sessionModel->rowCount()) {
		this->m_sessionModel->item(index, 0)->setText(QString("%0").arg(id));
		this->m_sessionModel->item(index, 1)->setText(QString("%0").arg(session->getSessionName()));
		this->m_sessionModel->item(index, 2)->setText(QString("%0").arg(session->getProcessID()));
		this->m_sessionModel->item(index, 3)->setText(QString("%0").arg(session->getThreadID()));
		this->m_sessionModel->item(index, 4)->setText(QString("%0").arg(session->getPeerAddress()));
	}
}