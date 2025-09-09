/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QtProperty;
class QDialogButtonBox;
class QtTreePropertyBrowser;
class QLabel;
QT_END_NAMESPACE

class SettingDialog : public QDialog
{
	Q_OBJECT

private slots:
	void scriptEditButtonClicked(QtProperty *);
	void valueChanged(QtProperty *property, const QVariant &value);
	void clearMessage() ;

private:
	void _initProperty(void);
	void _setWarningText(const QString& message);

private:
	QtTreePropertyBrowser* m_propertyBrowser;
	QDialogButtonBox* m_dlgButtons;
	QLabel* m_warningLabel;

public:
	SettingDialog(QWidget *parent = nullptr);
	~SettingDialog();
};
