/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_SettingDialog.h"
#include "AT4_System.h"
#include "AT4_Config.h"
#include "AT4_LuaHighlighter.h"
#include "AT4_Filter.h"

//--------------------------------------------------------------------------------------------
CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
	lineNumberArea = new LineNumberArea(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
}

//--------------------------------------------------------------------------------------------
int CodeEditor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) {
		max /= 10;
		++digits;
	}

	int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

	return space;
}

//--------------------------------------------------------------------------------------------
void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

//--------------------------------------------------------------------------------------------
void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

//--------------------------------------------------------------------------------------------
void CodeEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

//--------------------------------------------------------------------------------------------
void CodeEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::yellow).lighter(160);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

//--------------------------------------------------------------------------------------------
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(lineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int)blockBoundingRect(block).height();

	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
				Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();
		++blockNumber;
	}
}

//--------------------------------------------------------------------------------------------
SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent)
{
	QFont font;
	font.setFamily("Courier");
	font.setFixedPitch(true);
	font.setPointSize(10);

	m_editor = new CodeEditor;
	m_editor->setFont(font);

	m_highlighter = new LuaHighlighter(m_editor->document());

	Config* config = System::getSingleton()->getConfig();
	m_editor->setPlainText(config->getFilterScript());

	m_defaultButton = new QPushButton(tr("Default"));
	connect(m_defaultButton, SIGNAL(clicked()), this, SLOT(resetScript()));

	m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok
		| QDialogButtonBox::Cancel);

	connect(m_buttons, &QDialogButtonBox::accepted, this, &SettingDialog::verify);
	connect(m_buttons, &QDialogButtonBox::rejected, this, &SettingDialog::reject);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(m_editor);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addWidget(m_defaultButton);
	buttonLayout->addWidget(m_buttons);

	mainLayout->addLayout(buttonLayout);

	setLayout(mainLayout);
	resize(1024, 512);
	setWindowTitle(tr("AxTrace Setting Dialog"));
}

//--------------------------------------------------------------------------------------------
SettingDialog::~SettingDialog()
{

}

//--------------------------------------------------------------------------------------------
void SettingDialog::verify()
{
	QString errorMsg;

	if (!Filter::tryLoadScript(m_editor->toPlainText().toUtf8().toStdString().c_str(), errorMsg))
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
	accept();
}

//--------------------------------------------------------------------------------------------
void SettingDialog::resetScript()
{
	if (QMessageBox::Yes != QMessageBox::question(this,
		tr("Axtrace 4"), tr("Do you want reset filter script to default?"),
		QMessageBox::Yes | QMessageBox::No))
	{
		return;
	}

	m_editor->setPlainText(System::getSingleton()->getConfig()->getDefaultFilterScript());
}
