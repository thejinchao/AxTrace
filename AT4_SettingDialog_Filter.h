/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include "AT4_SettingDialog.h"

class CodeEditor;
class LuaHighlighter;

class SettingDialog_Filter : public QWidget, public SettingDialog::SubInterface
{
	Q_OBJECT

public:
	const QString& getScript(void) const { return m_script; }

public:
	virtual bool onVerify(void);
	virtual void onReset(void);

private:
	enum { DEFAULT_FONT_SIZE=10 };
	CodeEditor*	m_editor;
	LuaHighlighter* m_highlighter;
	QString m_script;

public:
	explicit SettingDialog_Filter(QWidget *parent = 0);
};
