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
#include "AT4_MainWindow.h"

//--------------------------------------------------------------------------------------------
ScriptEditorDialog::ScriptEditorDialog(QWidget *parent)
	: QDialog(parent)
{
	QFont font;
	font.setFamily("Courier");
	font.setFixedPitch(true);
	font.setPointSize(DEFAULT_FONT_SIZE);

	m_editor = new CodeEditor;
	m_editor->setFont(font);

	m_defaultButton = new QPushButton(tr("Default"));
	connect(m_defaultButton, SIGNAL(clicked()), this, SLOT(onResetButton()));

	m_dlgButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(m_dlgButtons, &QDialogButtonBox::accepted, this, &ScriptEditorDialog::verify);
	connect(m_dlgButtons, &QDialogButtonBox::rejected, this, &ScriptEditorDialog::reject);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_editor);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(m_defaultButton);
	buttonLayout->addWidget(m_dlgButtons);

	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);
	resize(1024, 512);
	setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
	setWindowTitle(tr("AxTrace Filter Script"));
}

//--------------------------------------------------------------------------------------------
ScriptEditorDialog_Filter::ScriptEditorDialog_Filter(QWidget *parent)
	: ScriptEditorDialog(parent)
{
	m_highlighter = new LuaHighlighter(m_editor->document());

	Config* config = System::getSingleton()->getConfig();
	m_editor->setPlainText(config->getFilterScript());
}

//--------------------------------------------------------------------------------------------
void ScriptEditorDialog_Filter::verify(void)
{
	QString errorMsg;
	std::string plainText = m_editor->toPlainText().toUtf8().toStdString();
	if (!Filter::tryLoadScript(plainText.c_str(), errorMsg))
	{
		QMessageBox::critical(this, tr("LoadScript Error"), errorMsg, QMessageBox::Ok);
		return;
	}

	if (QMessageBox::Yes != QMessageBox::warning(this,
		tr("Compile Script Success!"), tr("Do you want reload filter script now?"),
		QMessageBox::Yes | QMessageBox::No))
	{
		return;
	}

	m_script = m_editor->toPlainText();

	//reload now
	bool reloadSuccess = System::getSingleton()->getFilter()->reloadScript(plainText.c_str());
	Q_ASSERT(reloadSuccess);

	System::getSingleton()->getConfig()->setFilterScript(m_script);
	accept();
}

//--------------------------------------------------------------------------------------------
void ScriptEditorDialog::onResetButton()
{
	reset();
}

//--------------------------------------------------------------------------------------------
void ScriptEditorDialog_Filter::reset(void)
{
	if (QMessageBox::Yes != QMessageBox::question(this,
		tr("Axtrace 4"), tr("Do you want reset filter script to default?"),
		QMessageBox::Yes | QMessageBox::No))
	{
		return;
	}

	m_editor->setPlainText(System::getSingleton()->getConfig()->getDefaultFilterScript());
}

//--------------------------------------------------------------------------------------------
ScriptEditorDialog_LogParser::ScriptEditorDialog_LogParser(QWidget *parent)
	: ScriptEditorDialog(parent)
{
	Config* config = System::getSingleton()->getConfig();
	m_editor->setPlainText(config->getLoaParserScript());
}

//--------------------------------------------------------------------------------------------
void ScriptEditorDialog_LogParser::verify(void)
{
	if (System::getSingleton()->getMainWindow()->getLogChildCounts() > 0) 
	{
		QMessageBox::critical(this, tr("Axtrace 4"), tr("Close all log window first!"), QMessageBox::Ok);
		return;
	}

	m_script = m_editor->toPlainText();

	QString errorMsg;
	LogParser::DefineMap logParserDefineMap;
	if (!LogParser::tryLoadParserScript(m_script, errorMsg, logParserDefineMap))
	{
		QMessageBox::critical(this, tr("Axtrace 4"), tr("Load log parser script error:%1").arg(errorMsg), QMessageBox::Ok);
		return;
	}
	System::getSingleton()->getConfig()->setLogParserScript(m_script);
	accept();
}

//--------------------------------------------------------------------------------------------
void ScriptEditorDialog_LogParser::reset(void)
{
	if (QMessageBox::Yes != QMessageBox::question(this,
		tr("Axtrace 4"), tr("Do you want reset log parser script to default?"),
		QMessageBox::Yes | QMessageBox::No))
	{
		return;
	}

	m_editor->setPlainText(System::getSingleton()->getConfig()->getDefaultLogParserScript());
}

