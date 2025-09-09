/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#include "stdafx.h"
#include "AT4_SettingDialog.h"
#include "AT4_ScriptEditDialog.h"
#include "AT4_ExtensionProperty.h"
#include "AT4_ScriptVariant.h"
#include "AT4_System.h"
#include "AT4_Config.h"
#include "AT4_Incoming.h"

//--------------------------------------------------------------------------------------------
SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent)
{
	_initProperty();

	m_warningLabel = new QLabel;
	QFont font = m_warningLabel->font();
	font.setBold(true);
	m_warningLabel->setFont(font);

	m_dlgButtons = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(m_dlgButtons, &QDialogButtonBox::rejected, this, &SettingDialog::reject);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_propertyBrowser);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(m_warningLabel);
	buttonLayout->addWidget(m_dlgButtons);

	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);
	resize(480, 600);
	setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
	setWindowTitle(tr("AxTrace Setting Dialog"));
}

//--------------------------------------------------------------------------------------------
SettingDialog::~SettingDialog()
{

}

//--------------------------------------------------------------------------------------------
void SettingDialog::_initProperty(void)
{
	Config* config = System::getSingleton()->getConfig();
	QtVariantPropertyManager *variantManager = new VariantPropertyManager();
	connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
		this, SLOT(valueChanged(QtProperty *, const QVariant &)));

	//==== General Group =====
	QtProperty *generalGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(),QString("General"));

	QtVariantProperty *listenPort = variantManager->addProperty(QMetaType::Int, QString("ListenPort"));
	listenPort->setValue(config->getListenPort());
	listenPort->setAttribute(QLatin1String("minimum"), Config::LISTEN_PORT_MIN);
	listenPort->setAttribute(QLatin1String("maximum"), Config::LISTEN_PORT_MAX);
	generalGroup->addSubProperty(listenPort);

	QtVariantProperty *filterScript = variantManager->addProperty(ScriptVariant::Type, QString("FilterScript"));
	filterScript->setValue(ScriptVariant("Filter", ""));
	generalGroup->addSubProperty(filterScript);

	//==== Log Group =====
	QtProperty *logGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), QString("Log"));

	QtVariantProperty *logParser = variantManager->addProperty(ScriptVariant::Type, QString("ParserScript"));
	logParser->setValue(ScriptVariant("Parser", ""));
	logGroup->addSubProperty(logParser);

	QtVariantProperty* maxLogCounts = variantManager->addProperty(QMetaType::Int, QString("MaxLogCounts"));
	maxLogCounts->setValue(config->getMaxLogCounts());
	maxLogCounts->setAttribute(QLatin1String("minimum"), Config::MAX_LOG_COUNTS_RANGE_MIN);
	maxLogCounts->setAttribute(QLatin1String("maximum"), Config::MAX_LOG_COUNTS_RANGE_MAX);
	logGroup->addSubProperty(maxLogCounts);

	//==== 2D Actor Group
	QtProperty *actor2DGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), QString("2D Actor"));

	QtVariantProperty* maxActorLogCounts = variantManager->addProperty(QMetaType::Int, QString("MaxActorLogCounts"));
	maxActorLogCounts->setValue(config->getMaxActorLogCounts());
	maxActorLogCounts->setAttribute(QLatin1String("minimum"), Config::MAX_ACTOR_LOG_COUNTS_RANGE_MIN);
	maxActorLogCounts->setAttribute(QLatin1String("maximum"), Config::MAX_ACTOR_LOG_COUNTS_RANGE_MAX);
	actor2DGroup->addSubProperty(maxActorLogCounts);

	QtVariantProperty* maxActorTailCounts = variantManager->addProperty(QMetaType::Int, QString("MaxActorTailCounts"));
	maxActorTailCounts->setValue(config->getMaxActorTailCounts());
	maxActorTailCounts->setAttribute(QLatin1String("minimum"), Config::MAX_ACTOR_TAIL_COUNTS_RANGE_MIN);
	maxActorTailCounts->setAttribute(QLatin1String("maximum"), Config::MAX_ACTOR_TAIL_COUNTS_RANGE_MAX);
	actor2DGroup->addSubProperty(maxActorTailCounts);

	VariantEditorFactory *variantFactory = new VariantEditorFactory();
	connect(variantFactory, SIGNAL(scriptEditButtonClicked(QtProperty *)), 
		this, SLOT(scriptEditButtonClicked(QtProperty *)));

	//Show dialog
	m_propertyBrowser = new QtTreePropertyBrowser();
	m_propertyBrowser->setFactoryForManager(variantManager, variantFactory);
	m_propertyBrowser->addProperty(generalGroup);
	m_propertyBrowser->addProperty(logGroup);
	m_propertyBrowser->addProperty(actor2DGroup);
	m_propertyBrowser->setPropertiesWithoutValueMarked(true);
	m_propertyBrowser->setRootIsDecorated(false);
}

//--------------------------------------------------------------------------------------------
void SettingDialog::_setWarningText(const QString& message)
{
	m_warningLabel->setText(message);
	QTimer::singleShot(2 * 1000, this, SLOT(clearMessage()));
}

//--------------------------------------------------------------------------------------------
void SettingDialog::scriptEditButtonClicked(QtProperty* property)
{
	QtVariantProperty *filterScript = static_cast<QtVariantProperty*>(property);
	ScriptVariant scriptVariant = filterScript->value().value<ScriptVariant>();

	if (scriptVariant.type == "Filter")
	{
		ScriptEditorDialog_Filter dlg;
		dlg.exec();
	}
	else if (scriptVariant.type == "Parser")
	{
		ScriptEditorDialog_LogParser dlg;
		dlg.exec();
	}
}

//--------------------------------------------------------------------------------------------
void SettingDialog::valueChanged(QtProperty *property, const QVariant &value)
{
	QString propertyName = property->propertyName();
	Config* config = System::getSingleton()->getConfig();

	if (propertyName == "ListenPort") {
		qint32 listenPort = value.toInt();
		if (config->getListenPort() == listenPort) return;

		SessionManager* sessionManager = System::getSingleton()->getSessionManager();
		if (sessionManager->getSessionCounts() > 0)
		{
			((QtVariantProperty *)property)->setValue(config->getListenPort());
			_setWarningText(tr("Disconnect all connection first!"));
			return;
		}

		Incoming* incoming = System::getSingleton()->getIncoming();
		Q_ASSERT(incoming);

		//shutdown current network
		incoming->close();

		//set new listen port and restart network
		config->setListenPort(listenPort);
		incoming->init(listenPort);
	}
	else if (propertyName == "MaxLogCounts") {
		qint32 maxLogCounts = value.toInt();
		config->setMaxLogCounts(maxLogCounts);
	}
	else if (propertyName == "MaxActorLogCounts") {
		qint32 maxActorLogCounts = value.toInt();
		config->setMaxActorLogCounts(maxActorLogCounts);
	}
	else if (propertyName == "MaxActorTailCounts") {
		qint32 maxActorTailCounts = value.toInt();
		config->setMaxActorTailCounts(maxActorTailCounts);
	}
}

//--------------------------------------------------------------------------------------------
void SettingDialog::clearMessage()
{
	m_warningLabel->setText("");
}
