/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
QT_END_NAMESPACE

class SettingDialog : public QDialog
{
	Q_OBJECT
public:
	class SubInterface
	{
	public:
		virtual bool onVerify(void) = 0;
		virtual void onReset(void) = 0;
	};

public slots:
	void verify();
	void reset();

private:
	QTabWidget*			m_tabWidget;
	QPushButton*		m_defaultButton;
	QDialogButtonBox*	m_dlgButtons;

public:
	SettingDialog(QWidget *parent = nullptr);
	~SettingDialog();
};
