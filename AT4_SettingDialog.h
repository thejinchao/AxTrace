/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include <QDialog>
#include <QPlainTextEdit>

class LuaHighlighter;

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
QT_END_NAMESPACE


class CodeEditor : public QPlainTextEdit
{
	Q_OBJECT

public:
	CodeEditor(QWidget *parent = 0);

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();

protected:
	void resizeEvent(QResizeEvent *event) override;

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &, int);

private:
	QWidget *lineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(CodeEditor *editor) : QWidget(editor) {
		codeEditor = editor;
	}

	QSize sizeHint() const override {
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) override {
		codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	CodeEditor *codeEditor;
};

class SettingDialog : public QDialog
{
	Q_OBJECT

public:
	const QString& getScript(void) const { return m_script; }

public slots:
	void verify();
	void resetScript();

private:
	CodeEditor*	m_editor;
	QDialogButtonBox* m_buttons;
	QPushButton* m_defaultButton;
	LuaHighlighter* m_highlighter;
	QString m_script;

public:
	SettingDialog(QWidget *parent = nullptr);
	~SettingDialog();
};
