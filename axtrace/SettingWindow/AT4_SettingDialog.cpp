/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#include "stdafx.h"
#include "AT4_SettingDialog.h"
#include "ScriptEditorWindow/AT4_ScriptEditDialog.h"
#include "AT4_System.h"
#include "AT4_Config.h"
#include "AT4_Incoming.h"
#include "Session/AT4_Session.h"

//--------------------------------------------------------------------------------------------
SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent)
{
	m_warningLabel = new QLabel;
	QFont font = m_warningLabel->font();
	font.setBold(true);
	m_warningLabel->setFont(font);

	m_dlgButtons = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(m_dlgButtons, &QDialogButtonBox::rejected, this, &SettingDialog::reject);

	QVBoxLayout *mainLayout = new QVBoxLayout;

	//---------------------------
	Config* config = System::getSingleton()->getConfig();
	const int FIXED_LABEL_WIDTH = 150;

#define ADD_SPINBOX_FIELD(layout, controlName, minValue, maxValue, initValue) \
	{ \
		QLabel* label = new QLabel(#controlName); \
		label->setFixedWidth(FIXED_LABEL_WIDTH); \
		label->setAlignment(Qt::AlignRight | Qt::AlignVCenter); \
		QSpinBox* field = new QSpinBox(); \
		field->setRange(minValue, maxValue); \
		field->setValue(initValue); \
		connect(field, SIGNAL(editingFinished()), this, SLOT(on##controlName##Changed())); \
		layout->addRow(label, field); \
		m_ctl##controlName = field; \
	}

#define ADD_PUSHBUTTON_FIELD(layout, controlName, buttonText) \
	{ \
		QLabel* label = new QLabel(#controlName); \
		label->setFixedWidth(FIXED_LABEL_WIDTH); \
		label->setAlignment(Qt::AlignRight | Qt::AlignVCenter); \
		QPushButton* field = new QPushButton(buttonText); \
		connect(field, SIGNAL(clicked()), this, SLOT(on##controlName##ButtonClicked())); \
		layout->addRow(label, field); \
	}


	{
		QGroupBox* generalGroup = new QGroupBox("General");
		QFormLayout* generalLayout = new QFormLayout();
		generalLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
		generalLayout->setSpacing(10);

		ADD_SPINBOX_FIELD(generalLayout, ListenPort,
			Config::LISTEN_PORT_MIN, Config::LISTEN_PORT_MAX,
			config->getListenPort())

		ADD_PUSHBUTTON_FIELD(generalLayout, FilterScript, "Edit Filter Script...");

		generalGroup->setLayout(generalLayout);
		mainLayout->addWidget(generalGroup);
	}

	//-----------
	{
		QGroupBox* logGroup = new QGroupBox("Log");
		QFormLayout* logLayout = new QFormLayout();
		logLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
		logLayout->setSpacing(10);

		ADD_PUSHBUTTON_FIELD(logLayout, ParserScript, "Edit Parser Script...");

		ADD_SPINBOX_FIELD(logLayout, MaxLogCounts,
			Config::MAX_LOG_COUNTS_RANGE_MIN, Config::MAX_LOG_COUNTS_RANGE_MAX,
			config->getMaxLogCounts());

		logGroup->setLayout(logLayout);
		mainLayout->addWidget(logGroup);
	}

	//-----------
	{
		QGroupBox* twoDActorGroup = new QGroupBox("2D Actor");
		QFormLayout* twoDActorLayout = new QFormLayout();
		twoDActorLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
		twoDActorLayout->setSpacing(10);

		ADD_SPINBOX_FIELD(twoDActorLayout, MaxActorLogCounts,
			Config::MAX_ACTOR_LOG_COUNTS_RANGE_MIN, Config::MAX_ACTOR_LOG_COUNTS_RANGE_MAX,
			config->getMaxActorLogCounts());

		ADD_SPINBOX_FIELD(twoDActorLayout, MaxActorTailCounts,
			Config::MAX_ACTOR_TAIL_COUNTS_RANGE_MIN, Config::MAX_ACTOR_TAIL_COUNTS_RANGE_MAX,
			config->getMaxActorTailCounts())

		twoDActorGroup->setLayout(twoDActorLayout);
		mainLayout->addWidget(twoDActorGroup);
	}

	//---------------------------

	mainLayout->addStretch();

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(m_warningLabel);
	buttonLayout->addWidget(m_dlgButtons);

	mainLayout->addLayout(buttonLayout);

	QPushButton* closeButton = m_dlgButtons->button(QDialogButtonBox::Close);
	QTimer::singleShot(0, [closeButton]() {
		closeButton->setFocus();
	});

	setLayout(mainLayout);
	resize(480, 300);
	setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
	setWindowTitle(tr("AxTrace Setting Dialog"));
}

//--------------------------------------------------------------------------------------------
SettingDialog::~SettingDialog()
{

}

//--------------------------------------------------------------------------------------------
void SettingDialog::_setWarningText(const QString& message)
{
	m_warningLabel->setText(message);
	QTimer::singleShot(2 * 1000, this, SLOT(clearMessage()));
}

//--------------------------------------------------------------------------------------------
void SettingDialog::onListenPortChanged()
{
	Config* config = System::getSingleton()->getConfig();
	qint32 value = m_ctlListenPort->value();
	if (config->getListenPort() == value) return;

	SessionManager* sessionManager = System::getSingleton()->getSessionManager();
	if (sessionManager->getSessionCounts() > 0)
	{
		m_ctlListenPort->setValue(config->getListenPort());
		_setWarningText(tr("Disconnect all connection first!"));
		return;
	}

	Incoming* incoming = System::getSingleton()->getIncoming();
	Q_ASSERT(incoming);

	//shutdown current network
	incoming->close();

	//set new listen port and restart network
	config->setListenPort(value);
	incoming->init(value);
}

//--------------------------------------------------------------------------------------------
void SettingDialog::onMaxLogCountsChanged()
{
	Config* config = System::getSingleton()->getConfig();
	qint32 maxLogCounts = m_ctlMaxLogCounts->value();

	config->setMaxLogCounts(maxLogCounts);
}

//--------------------------------------------------------------------------------------------
void SettingDialog::onMaxActorLogCountsChanged()
{
	Config* config = System::getSingleton()->getConfig();
	qint32 maxActorLogCounts = m_ctlMaxActorLogCounts->value();

	config->setMaxActorLogCounts(maxActorLogCounts);
}

//--------------------------------------------------------------------------------------------
void SettingDialog::onMaxActorTailCountsChanged()
{
	Config* config = System::getSingleton()->getConfig();
	qint32 maxActorTailCounts = m_ctlMaxActorTailCounts->value();

	config->setMaxActorTailCounts(maxActorTailCounts);
}

//--------------------------------------------------------------------------------------------
void SettingDialog::onFilterScriptButtonClicked()
{
	ScriptEditorDialog_Filter dlg;
	dlg.exec();
}

//--------------------------------------------------------------------------------------------
void SettingDialog::onParserScriptButtonClicked()
{
	ScriptEditorDialog_LogParser dlg;
	dlg.exec();
}

//--------------------------------------------------------------------------------------------
void SettingDialog::clearMessage()
{
	m_warningLabel->setText("");
}
