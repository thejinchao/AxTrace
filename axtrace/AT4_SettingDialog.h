/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QtProperty;
class QDialogButtonBox;
class QtTreePropertyBrowser;
QT_END_NAMESPACE

class SettingDialog : public QDialog
{
	Q_OBJECT

private slots:
	void scriptEditButtonClicked(QtProperty *);
	void valueChanged(QtProperty *property, const QVariant &value);

private:
	void _initProperty(void);
	void _addProperty(QtProperty *property, const QString &id);

private:
	QtTreePropertyBrowser* m_propertyBrowser;
	QDialogButtonBox* m_dlgButtons;

public:
	SettingDialog(QWidget *parent = nullptr);
	~SettingDialog();
};
