/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include "AT4_SettingDialog.h"

class CodeEditor;
class LuaHighlighter;

class ScriptEditorDialog : public QDialog
{
	Q_OBJECT

public:
	const QString& getScript(void) const { return m_script; }

public:
	virtual void verify(void) = 0;
	virtual void reset(void) = 0;

private Q_SLOTS:
	void onResetButton();

protected:
	enum { DEFAULT_FONT_SIZE=10 };
	CodeEditor*	m_editor;
	QString m_script;

	QPushButton* m_defaultButton;
	QDialogButtonBox* m_dlgButtons;

public:
	explicit ScriptEditorDialog(QWidget *parent = 0);
};

class ScriptEditorDialog_Filter : public ScriptEditorDialog
{
public:
	ScriptEditorDialog_Filter(QWidget *parent = 0);

public:
	virtual void verify(void);
	virtual void reset(void);

private:
	LuaHighlighter* m_highlighter;
};

class ScriptEditorDialog_LogParser : public ScriptEditorDialog
{
public:
	ScriptEditorDialog_LogParser(QWidget *parent = 0);

public:
	virtual void verify(void);
	virtual void reset(void);
};

