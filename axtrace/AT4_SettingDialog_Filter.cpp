/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_SettingDialog_Filter.h"
#include "AT4_System.h"
#include "AT4_Config.h"
#include "AT4_Filter.h"
#include "AT4_LuaHighlighter.h"
#include "AT4_CodeEditor.h"

//--------------------------------------------------------------------------------------------
SettingDialog_Filter::SettingDialog_Filter(QWidget *parent)
	: QWidget(parent)
{
	QFont font;
	font.setFamily("Courier");
	font.setFixedPitch(true);
	font.setPointSize(DEFAULT_FONT_SIZE);

	m_editor = new CodeEditor;
	m_editor->setFont(font);

	m_highlighter = new LuaHighlighter(m_editor->document());

	Config* config = System::getSingleton()->getConfig();
	m_editor->setPlainText(config->getFilterScript());

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_editor);

	setLayout(mainLayout);
}

//--------------------------------------------------------------------------------------------
bool SettingDialog_Filter::onVerify(void)
{
	QString errorMsg;
	std::string plainText = m_editor->toPlainText().toUtf8().toStdString();
	if (!Filter::tryLoadScript(plainText.c_str(), errorMsg))
	{
		QMessageBox::critical(this, tr("LoadScript Error"), errorMsg, QMessageBox::Ok);
		return false;
	}

	if (QMessageBox::Yes != QMessageBox::warning(this,
		tr("Compile Script Success!"), tr("Do you want reload filter script now?"),
		QMessageBox::Yes | QMessageBox::No))
	{
		return false;
	}

	m_script = m_editor->toPlainText();

	//reload now
	bool reloadSuccess = System::getSingleton()->getFilter()->reloadScript(plainText.c_str());
	Q_ASSERT(reloadSuccess);

	System::getSingleton()->getConfig()->setFilterScript(m_script);
	return true;
}

//--------------------------------------------------------------------------------------------
void SettingDialog_Filter::onReset(void)
{
	if (QMessageBox::Yes != QMessageBox::question(this,
		tr("Axtrace 4"), tr("Do you want reset filter script to default?"),
		QMessageBox::Yes | QMessageBox::No))
	{
		return;
	}

	m_editor->setPlainText(System::getSingleton()->getConfig()->getDefaultFilterScript());
}
