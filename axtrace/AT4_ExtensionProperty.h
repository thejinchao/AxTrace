#pragma once

#include "qtvariantproperty.h"
#include "qteditorfactory.h"
#include "qttreepropertybrowser.h"

class ScriptPropertyManager;
class ScriptEditorFactory;

class VariantPropertyManager : public QtVariantPropertyManager
{
	Q_OBJECT
public:
	VariantPropertyManager(QObject *parent = 0);
	~VariantPropertyManager();

	ScriptPropertyManager* getScriptPropertyManager(void) {
		return m_scriptPropertyManager;
	}

	QString valueText(const QtProperty *property) const;

	virtual QVariant value(const QtProperty *property) const;
	virtual int valueType(int propertyType) const;
	virtual bool isPropertyTypeSupported(int propertyType) const;

public slots:
	virtual void setValue(QtProperty *property, const QVariant &val);

protected:
	virtual void initializeProperty(QtProperty *property);
	virtual void uninitializeProperty(QtProperty *property);

private:
	ScriptPropertyManager *m_scriptPropertyManager;
};


class VariantEditorFactory : public QtVariantEditorFactory
{
	Q_OBJECT
public:
	VariantEditorFactory(QObject *parent = 0);
	~VariantEditorFactory();

	QWidget* createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent);

private:
	ScriptEditorFactory *m_scriptEditorFactory;
};
