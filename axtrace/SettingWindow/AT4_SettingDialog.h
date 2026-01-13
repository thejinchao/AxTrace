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
class QSpinBox;
QT_END_NAMESPACE

class SettingDialog : public QDialog
{
	Q_OBJECT

private slots:
	void clearMessage() ;
	
	void onListenPortChanged();
	void onMaxLogCountsChanged();
	void onMaxActorLogCountsChanged();
	void onMaxActorTailCountsChanged();

	void onFilterScriptButtonClicked();
	void onParserScriptButtonClicked();
	

private:
	void _setWarningText(const QString& message);

private:
	QDialogButtonBox* m_dlgButtons;
	QLabel* m_warningLabel;

	QSpinBox* m_ctlListenPort;
	QSpinBox* m_ctlMaxLogCounts;
	QSpinBox* m_ctlMaxActorLogCounts;
	QSpinBox* m_ctlMaxActorTailCounts;

public:
	SettingDialog(QWidget *parent = nullptr);
	~SettingDialog();
};
