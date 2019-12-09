#pragma once

#include <QVariant>
#include <QMetaType>
#include "qtpropertybrowser.h"

struct ScriptVariant
{
	enum {
		Type = QVariant::UserType + 100
	};

	operator QVariant() const {
		return QVariant::fromValue(*this);
	}

	ScriptVariant() {}
	ScriptVariant(const QString& _type, const QString& _value) : type(_type), value(_value){ }

	QString type;
	QString value;
};

Q_DECLARE_METATYPE(ScriptVariant)

class ScriptPropertyManagerPrivate;

class ScriptPropertyManager : public QtAbstractPropertyManager
{
	Q_OBJECT
public:
	ScriptPropertyManager(QObject *parent = 0);
	~ScriptPropertyManager();

	ScriptVariant value(const QtProperty *property) const;
	QString valueText(const QtProperty *property) const;

	virtual void initializeProperty(QtProperty *property);
	virtual void uninitializeProperty(QtProperty *property);

public Q_SLOTS:
	void setValue(QtProperty *property, const ScriptVariant &val);
Q_SIGNALS:
	void valueChanged(QtProperty *property, const ScriptVariant &val);

private:
	ScriptPropertyManagerPrivate *d_ptr;
	Q_DECLARE_PRIVATE(ScriptPropertyManager)
	Q_DISABLE_COPY(ScriptPropertyManager)
};


class ScriptEditorFactoryPrivate;

class ScriptEditorFactory : public QtAbstractEditorFactory<ScriptPropertyManager>
{
	Q_OBJECT
public:
	ScriptEditorFactory(QObject *parent = 0);
	~ScriptEditorFactory();

	QWidget *createEditor(ScriptPropertyManager *manager, QtProperty *property, QWidget *parent);

public Q_SLOTS:
	void editButtonClicked(QWidget* editor);
	void slotEditorDestroyed(QObject*);

protected:
	void connectPropertyManager(ScriptPropertyManager *manager);
	void disconnectPropertyManager(ScriptPropertyManager *manager);

Q_SIGNALS:
	void editButtonClicked(QtProperty*);

private:
	ScriptEditorFactoryPrivate *d_ptr;
	Q_DECLARE_PRIVATE(ScriptEditorFactory)
	Q_DISABLE_COPY(ScriptEditorFactory)
};
