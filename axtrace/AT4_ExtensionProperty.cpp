/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#include "stdafx.h"
#include "AT4_ExtensionProperty.h"
#include "AT4_ScriptVariant.h"

VariantPropertyManager::VariantPropertyManager(QObject *parent)
	: QtVariantPropertyManager(parent)
{
	m_scriptPropertyManager = new ScriptPropertyManager(this);
}

VariantPropertyManager::~VariantPropertyManager()
{

}

QString VariantPropertyManager::valueText(const QtProperty *property) const
{
	if (propertyType(property) == ScriptVariant::Type) {
		return m_scriptPropertyManager->valueText(property);
	}

	return QtVariantPropertyManager::valueText(property);
}

QVariant VariantPropertyManager::value(const QtProperty *property) const
{
	if (propertyType(property) == ScriptVariant::Type) {
		return m_scriptPropertyManager->value(property);
	}
	return QtVariantPropertyManager::value(property);
}

int VariantPropertyManager::valueType(int propertyType) const
{
	if (propertyType == ScriptVariant::Type) {
		return ScriptVariant::Type;
	}

	return QtVariantPropertyManager::valueType(propertyType);
}

bool VariantPropertyManager::isPropertyTypeSupported(int propertyType) const
{
	if (propertyType == ScriptVariant::Type) {
		return true;
	}

	return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

void VariantPropertyManager::setValue(QtProperty *property, const QVariant &val)
{
	if (propertyType(property) == ScriptVariant::Type) {
		m_scriptPropertyManager->setValue(property, val.value<ScriptVariant>());
		return;
	}

	QtVariantPropertyManager::setValue(property, val);
}

void VariantPropertyManager::initializeProperty(QtProperty *property)
{
	if (propertyType(property) == ScriptVariant::Type) {
		m_scriptPropertyManager->initializeProperty(property);
		return;
	}

	QtVariantPropertyManager::initializeProperty(property);
}

void VariantPropertyManager::uninitializeProperty(QtProperty *property)
{
	if (propertyType(property) == ScriptVariant::Type) {
		m_scriptPropertyManager->uninitializeProperty(property);
		return;
	}

	QtVariantPropertyManager::uninitializeProperty(property);
}

VariantEditorFactory::VariantEditorFactory(QObject *parent /*= 0*/)
	: QtVariantEditorFactory(parent)
{
	m_scriptEditorFactory = new ScriptEditorFactory(this);
	connect(m_scriptEditorFactory, SIGNAL(editButtonClicked(QtProperty *)), this, SLOT(editButtonClicked(QtProperty *)));
}

VariantEditorFactory::~VariantEditorFactory()
{

}

QWidget* VariantEditorFactory::createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent)
{
	const int propType = manager->propertyType(property);
	if (propType == ScriptVariant::Type)
	{
		QtAbstractPropertyManager* mgr = property->propertyManager();
		VariantPropertyManager* vmgr = (VariantPropertyManager*)mgr;
		return m_scriptEditorFactory->createEditor(vmgr->getScriptPropertyManager(), property, parent);
	}

	return QtVariantEditorFactory::createEditor(manager, property, parent);
}

void VariantEditorFactory::editButtonClicked(QtProperty * property)
{
	emit scriptEditButtonClicked(property);
}
