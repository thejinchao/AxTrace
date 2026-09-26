/***************************************************

                AXIA|Trace4

    (C) Copyright thecodeway.com 2026
***************************************************/
#include "stdafx.h"
#include "AT4_SearchWindow.h"
#include "ChildWindow/AT4_ChildInterface.h"
#include "AT4_System.h"
#include "AT4_MainWindow.h"

//--------------------------------------------------------------------------------------------
SearchWindow::SearchWindow(QWidget* parent)
    : QDialog(parent)
    , m_findEdit(new QLineEdit(this))
    , m_matchCaseCheck(new QCheckBox(tr("Match Case"), this))
    , m_wholeWordsCheck(new QCheckBox(tr("Whole words"), this))
{
    setWindowTitle(tr("Search"));
    setModal(false);
    setAttribute(Qt::WA_DeleteOnClose);

    auto* findLabel = new QLabel(tr("Find what:"), this);
    findLabel->setBuddy(m_findEdit);

    auto* selectAllButton = new QPushButton(tr("Select All"), this);
    auto* closeButton = new QPushButton(tr("Close"), this);

    auto* layout = new QGridLayout(this);
    layout->addWidget(findLabel, 0, 0);
    layout->addWidget(m_findEdit, 0, 1);
    layout->addWidget(selectAllButton, 0, 2);
    layout->addWidget(m_matchCaseCheck, 1, 0, 1, 2);
    layout->addWidget(closeButton, 1, 2);
    layout->addWidget(m_wholeWordsCheck, 2, 0, 1, 2);
    layout->setColumnStretch(1, 1);

    const QSize hint = sizeHint();
    resize(hint.width() * 1.5, hint.height());

    connect(m_findEdit, &QLineEdit::textChanged, this, &SearchWindow::updateFindOptions);
    connect(m_matchCaseCheck, &QCheckBox::toggled, this, &SearchWindow::updateFindOptions);
    connect(m_wholeWordsCheck, &QCheckBox::toggled, this, &SearchWindow::updateFindOptions);
    connect(selectAllButton, &QPushButton::clicked, this, &SearchWindow::selectAllFindMatches);
    connect(closeButton, &QPushButton::clicked, this, &SearchWindow::close);
}

//--------------------------------------------------------------------------------------------
void SearchWindow::focusFindInput()
{
    m_findEdit->setFocus();
    m_findEdit->selectAll();
}

//--------------------------------------------------------------------------------------------
void SearchWindow::selectAllFindMatches()
{
    QTreeView* activeWindow = getActiveWindow();
    if (activeWindow == nullptr) return;

	IChildWindow* childWindow = qobject_cast<IChildWindow*>(activeWindow);
    if (childWindow == nullptr) return;

    auto model = activeWindow->model();
    if (model == nullptr) return;

	auto selectionModel = activeWindow->selectionModel();
    if (selectionModel == nullptr) return;

    QItemSelection matchingRows;
    if (!m_searchText.isEmpty() && m_searchExpression.isValid())
    {
        for (int row = 0; row < model->rowCount(); ++row) 
        {
            bool rowMatches = false;
            for (int column = 0; column < model->columnCount(); ++column)
            {
                auto matches = childWindow->searchMatchRanges(model->index(row, column), m_searchText, m_searchExpression);
                if (!matches.empty())
                {
                    rowMatches = true;
                    break;
                }
            }

            if (rowMatches) 
            {
                matchingRows.select(model->index(row, 0), model->index(row, model->columnCount() - 1));
            }
        }
    }

    selectionModel->clearSelection();
    selectionModel->select(matchingRows, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

//--------------------------------------------------------------------------------------------
void SearchWindow::updateFindOptions()
{
	m_searchText = m_findEdit->text();
    QString pattern = QRegularExpression::escape(m_searchText);
    if (m_wholeWordsCheck->isChecked()) 
    {
        pattern = QStringLiteral("(?<![\\p{L}\\p{N}_])(?:")
            + pattern
            + QStringLiteral(")(?![\\p{L}\\p{N}_])");
    }

    QRegularExpression::PatternOptions options = QRegularExpression::UseUnicodePropertiesOption;
    if (!m_matchCaseCheck->isChecked()) 
    {
        options |= QRegularExpression::CaseInsensitiveOption;
    }
    m_searchExpression = QRegularExpression(pattern, options);

    qobject_cast<QAbstractScrollArea*>(parent())->viewport()->update();
}

//--------------------------------------------------------------------------------------------
QBrush* SearchHighlightDelegate::s_highlightBrush = nullptr;

//--------------------------------------------------------------------------------------------
SearchHighlightDelegate::SearchHighlightDelegate(QTreeView* parent)
    : QStyledItemDelegate(parent)
{
	if (s_highlightBrush==nullptr)
	{
		s_highlightBrush = new QBrush(QColor(QStringLiteral("#f6b94d")));
	}
}

//--------------------------------------------------------------------------------------------
void SearchHighlightDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    SearchWindow* searchWindow = System::getSingleton()->getMainWindow()->getSearchWindow();
    if (searchWindow == nullptr || searchWindow->getActiveWindow() != parent())
    {
        QStyledItemDelegate::paint(painter, option, index);
		return;
    }

    IChildWindow* childWindow = qobject_cast<IChildWindow*>(parent());
    const auto matches = childWindow->searchMatchRanges(index, searchWindow->getSearchText(), searchWindow->getSearchExpression());
    if (matches.isEmpty()) 
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionViewItem textOption(option);
    initStyleOption(&textOption, index);

    const QString text = textOption.text;
    textOption.text.clear();
    QStyle* style = textOption.widget ? textOption.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &textOption, painter, textOption.widget);

    QTextLayout textLayout(text, textOption.font);

    const QColor textColor = textOption.palette.color(
        textOption.state.testFlag(QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text
    );

    QTextLayout::FormatRange baseFormat;
    baseFormat.start = 0;
    baseFormat.length = (int)text.size();
    baseFormat.format.setForeground(textColor);

    QList<QTextLayout::FormatRange> formats;
    formats.append(baseFormat);

    for (const auto& match : matches) 
    {
        QTextLayout::FormatRange matchFormat;
        matchFormat.start = match.first;
        matchFormat.length = match.second;
        matchFormat.format.setForeground(textColor);
        matchFormat.format.setBackground(*s_highlightBrush);
        formats.append(matchFormat);
    }
    textLayout.setFormats(formats);

    QTextOption layoutOption;
    layoutOption.setAlignment(textOption.displayAlignment & Qt::AlignHorizontal_Mask);
    layoutOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    textLayout.setTextOption(layoutOption);

    // Match QCommonStyle::viewItemDrawText: SE_ItemViewItemText is further
    // inset by PM_FocusFrameHMargin + 1 before text is drawn.
    const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &textOption, textOption.widget) + 1;
    const QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &textOption, textOption.widget).adjusted(textMargin, 0, -textMargin, 0);

    textLayout.beginLayout();
    qreal textHeight = 0.0;
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid()) break;

        line.setLineWidth(textRect.width());
        line.setPosition(QPointF(0.0, textHeight));
        textHeight += line.height();
    }
    textLayout.endLayout();

    qreal y = textRect.top();
    if (textOption.displayAlignment.testFlag(Qt::AlignVCenter)) {
        y += std::max(0.0, (textRect.height() - textHeight) / 2.0);
    }
    else if (textOption.displayAlignment.testFlag(Qt::AlignBottom)) {
        y += std::max(0.0, textRect.height() - textHeight);
    }
    painter->save();
    painter->setClipRect(textRect);
    textLayout.draw(painter, QPointF(textRect.left(), y));
    painter->restore();
}
