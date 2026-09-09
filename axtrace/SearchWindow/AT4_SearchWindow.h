/***************************************************

                AXIA|Trace4

    (C) Copyright thecodeway.com 2026
***************************************************/
#pragma once

#include <QStyledItemDelegate>

class QCheckBox;
class QCloseEvent;
class QLineEdit;
class IChildWindow;

class SearchWindow final : public QDialog
{
    Q_OBJECT

public:
    explicit SearchWindow(QWidget* parent = nullptr);

    void focusFindInput();

	void selectAllFindMatches();

    QTreeView* getActiveWindow() const { return qobject_cast<QTreeView*>(parent()); }

    const QString& getSearchText() const { return m_searchText; }

	const QRegularExpression& getSearchExpression() const { return m_searchExpression; }

private:
    void updateFindOptions();

    QTreeView* m_activeWindow;
    QString m_searchText;
    QRegularExpression m_searchExpression;

    QLineEdit* m_findEdit;
    QCheckBox* m_matchCaseCheck;
    QCheckBox* m_wholeWordsCheck;
};

class SearchHighlightDelegate final : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit SearchHighlightDelegate(QTreeView* parent);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
	static QBrush* s_highlightBrush;
};
