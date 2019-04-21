/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_LuaHighlighter.h"

//--------------------------------------------------------------------------------------------
LuaHighlighter::LuaHighlighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	keywordFormat.setForeground(Qt::darkBlue);
	keywordFormat.setFontWeight(QFont::Bold);
	QStringList keywordPatterns;
	keywordPatterns
		<< "\\band\\b"
		<< "\\bbreak\\b"
		<< "\\bdo\\b"
		<< "\\belse\\b"
		<< "\\belseif\\b"
		<< "\\bend\\b"
		<< "\\bfalse\\b"
		<< "\\bfor\\b"
		<< "\\bfunction\\b"
		<< "\\bif\\b"
		<< "\\bin\\b"
		<< "\\blocal\\b"
		<< "\\bnil\\b"
		<< "\\bnot\\b"
		<< "\\bor\\b"
		<< "\\brepeat\\b"
		<< "\\breturn\\b"
		<< "\\bthen\\b"
		<< "\\btrue\\b"
		<< "\\buntil\\b"
		<< "\\bwhile\\b";
	foreach(const QString &pattern, keywordPatterns) {
		rule.pattern = QRegularExpression(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	functionFormat.setFontItalic(true);
	functionFormat.setForeground(Qt::blue);
	rule.pattern = QRegularExpression("\\b:\\s*[A-Za-z0-9_]+(?=\\()");
	rule.format = functionFormat;
	highlightingRules.append(rule);

	keyFunctionFormat.setFontWeight(QFont::Bold);
	keyFunctionFormat.setForeground(Qt::darkMagenta);
	QStringList keyFunctions;
	keyFunctions
		<< "\\bonLogMessage(?=\\()\\b"
		<< "\\bonValueMessage(?=\\()\\b"
		<< "\\bonActor2DMessage(?=\\()\\b";
	foreach(const QString &pattern, keyFunctions) {
		rule.pattern = QRegularExpression(pattern);
		rule.format = keyFunctionFormat;
		highlightingRules.append(rule);
	}

	keyEnumFormat.setForeground(Qt::darkMagenta);
	QStringList keyEnums;
	keyEnums
		<< "\\bCOL_BLACK\\b"

		<< "\\bCOL_WHITE\\b"
		<< "\\bCOL_RED\\b"
		<< "\\bCOL_GREEN\\b"
		<< "\\bCOL_BLUE\\b"
		<< "\\bCOL_GRAY\\b"
		<< "\\bCOL_YELLOW\\b"
		<< "\\bCOL_ORANGE\\b"
		<< "\\bCOL_VIOLET\\b"
		<< "\\bAXT_TRACE\\b"
		<< "\\bAXT_DEBUG\\b"
		<< "\\bAXT_INFO\\b"
		<< "\\bAXT_WARN\\b"
		<< "\\bAXT_ERROR\\b"
		<< "\\bAXT_FATAL\\b"
		<< "\\bAXT_USERDEF\\b"
		<< "\\bACTOR_CIRCLE\\b"
		<< "\\bACTOR_QUAD\\b";
	foreach(const QString &pattern, keyEnums) {
		rule.pattern = QRegularExpression(pattern);
		rule.format = keyEnumFormat;
		highlightingRules.append(rule);
	}

	singleLineCommentFormat.setForeground(Qt::darkGreen);
	rule.pattern = QRegularExpression("--[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	multiLineCommentFormat.setForeground(Qt::darkGreen);

	commentStartExpression = QRegularExpression("--\\[\\[");
	commentEndExpression = QRegularExpression("--\\]\\]");
}

//--------------------------------------------------------------------------------------------
void LuaHighlighter::highlightBlock(const QString &text)
{
	foreach(const HighlightingRule &rule, highlightingRules) {
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
		while (matchIterator.hasNext()) {
			QRegularExpressionMatch match = matchIterator.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
		}
	}

	setCurrentBlockState(0);

	int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = text.indexOf(commentStartExpression);

	while (startIndex >= 0) {
		QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
		int endIndex = match.capturedStart();
		int commentLength = 0;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		}
		else {
			commentLength = endIndex - startIndex
				+ match.capturedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
	}
}
