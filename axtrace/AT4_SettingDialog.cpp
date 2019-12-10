/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_SettingDialog.h"
#include "AT4_SettingDialog_Filter.h"
#include "AT4_ExtensionProperty.h"
#include "AT4_ScriptVariant.h"
#include "AT4_System.h"
#include "AT4_Config.h"

//--------------------------------------------------------------------------------------------
SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent)
{
	_initProperty();

	m_dlgButtons = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(m_dlgButtons, &QDialogButtonBox::rejected, this, &SettingDialog::reject);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_propertyBrowser);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
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

	QtVariantProperty *filterScript = variantManager->addProperty(ScriptVariant::Type, QString("FilterScript"));
	filterScript->setValue(ScriptVariant("Filter", ""));
	generalGroup->addSubProperty(filterScript);

	//==== Log Group =====
	QtProperty *logGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), QString("Log"));

	QtVariantProperty *logParser = variantManager->addProperty(ScriptVariant::Type, QString("ParserScript"));
	logParser->setValue(ScriptVariant("Parser", ""));
	logGroup->addSubProperty(logParser);

	QtVariantProperty* maxLogCounts = variantManager->addProperty(QVariant::Int, QString("MaxLogCounts"));
	maxLogCounts->setValue(config->getMaxLogCounts());
	maxLogCounts->setAttribute(QLatin1String("minimum"), Config::MAX_LOG_COUNTS_RANGE_MIN);
	maxLogCounts->setAttribute(QLatin1String("maximum"), Config::MAX_LOG_COUNTS_RANGE_MAX);
	logGroup->addSubProperty(maxLogCounts);

	//==== 2D Actor Group
	QtProperty *actor2DGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), QString("2D Actor"));

	QtVariantProperty* maxActorLogCounts = variantManager->addProperty(QVariant::Int, QString("MaxActorLogCounts"));
	maxActorLogCounts->setValue(config->getMaxActorLogCounts());
	maxActorLogCounts->setAttribute(QLatin1String("minimum"), Config::MAX_ACTOR_LOG_COUNTS_RANGE_MIN);
	maxActorLogCounts->setAttribute(QLatin1String("maximum"), Config::MAX_ACTOR_LOG_COUNTS_RANGE_MAX);
	actor2DGroup->addSubProperty(maxActorLogCounts);

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
void SettingDialog::scriptEditButtonClicked(QtProperty* property)
{
	QtVariantProperty *filterScript = static_cast<QtVariantProperty*>(property);
	ScriptVariant scriptVariant = filterScript->value().value<ScriptVariant>();

	if (scriptVariant.type == "Filter")
	{
		SettingDialog_Filter dlg;
		dlg.exec();
	}
}

//--------------------------------------------------------------------------------------------
void SettingDialog::valueChanged(QtProperty *property, const QVariant &value)
{
	QString propertyName = property->propertyName();
	Config* config = System::getSingleton()->getConfig();

	if (propertyName == "MaxLogCounts") {
		qint32 maxLogCounts = value.toInt();
		config->setMaxLogCounts(maxLogCounts);
	}
	else if (propertyName == "MaxActorLogCounts") {
		qint32 maxActorLogCounts = value.toInt();
		config->setMaxActorLogCounts(maxActorLogCounts);
	}
}
