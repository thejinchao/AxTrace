/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_SettingDialog.h"
#include "AT4_SettingDialog_Filter.h"

//--------------------------------------------------------------------------------------------
SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent)
{
	m_tabWidget = new QTabWidget;
	m_tabWidget->addTab(new SettingDialog_Filter(), tr("Filter"));

	m_defaultButton = new QPushButton(tr("Default"));
	connect(m_defaultButton, SIGNAL(clicked()), this, SLOT(reset()));

	m_dlgButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(m_dlgButtons, &QDialogButtonBox::accepted, this, &SettingDialog::verify);
	connect(m_dlgButtons, &QDialogButtonBox::rejected, this, &SettingDialog::reject);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_tabWidget);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(m_defaultButton);
	buttonLayout->addWidget(m_dlgButtons);

	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);
	resize(1024, 512);
	setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
	setWindowTitle(tr("AxTrace Setting Dialog"));
}

//--------------------------------------------------------------------------------------------
SettingDialog::~SettingDialog()
{

}

//--------------------------------------------------------------------------------------------
void SettingDialog::verify()
{
	SubInterface* subInterface = dynamic_cast<SubInterface*>(m_tabWidget->currentWidget());
	if (subInterface->onVerify())
	{
		accept();
	}
}

//--------------------------------------------------------------------------------------------
void SettingDialog::reset()
{
	SubInterface* subInterface = dynamic_cast<SubInterface*>(m_tabWidget->currentWidget());
	subInterface->onReset();
}
