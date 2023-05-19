/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_ScriptVariant.h"
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QStyleOption>
#include <QPainter>

class ScriptPropertyManagerPrivate
{
	ScriptPropertyManager *q_ptr;
	Q_DECLARE_PUBLIC(ScriptPropertyManager)
public:

	struct Data
	{
		ScriptVariant data;

		Data() { }
		Data(const ScriptVariant& _data) : data(_data) {}
	};

	typedef QMap<const QtProperty *, Data> PropertyValueMap;
	QMap<const QtProperty *, Data> m_values;
};


ScriptPropertyManager::ScriptPropertyManager(QObject *parent)
	: QtAbstractPropertyManager(parent)
{
	d_ptr = new ScriptPropertyManagerPrivate;
	d_ptr->q_ptr = this;
}

/*!
	Destroys this manager, and all the properties it has created.
*/
ScriptPropertyManager::~ScriptPropertyManager()
{
	clear();
	delete d_ptr;
}

ScriptVariant ScriptPropertyManager::value(const QtProperty *property) const
{
	typedef ScriptPropertyManagerPrivate::PropertyValueMap PropertyToData;
	typedef typename PropertyToData::const_iterator PropertyToDataConstIterator;

	const PropertyToDataConstIterator it = d_ptr->m_values.constFind(property);
	if (it == d_ptr->m_values.constEnd()) {
		return ScriptVariant();
	}

	return it.value().data;
}

QString ScriptPropertyManager::valueText(const QtProperty *property) const
{
	const ScriptPropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.constFind(property);
	if (it == d_ptr->m_values.constEnd())
		return QString();
	return it.value().data.type;
}

void ScriptPropertyManager::setValue(QtProperty *property, const ScriptVariant &val)
{
	const ScriptPropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
	if (it == d_ptr->m_values.end())
		return;

	it.value() = val;

	emit propertyChanged(property);
	emit valueChanged(property, val);
}

void ScriptPropertyManager::initializeProperty(QtProperty *property)
{
	d_ptr->m_values[property] = ScriptPropertyManagerPrivate::Data();
}

void ScriptPropertyManager::uninitializeProperty(QtProperty *property)
{
	d_ptr->m_values.remove(property);
}

class ScriptEditWidget : public QWidget 
{
	Q_OBJECT

public:
	ScriptEditWidget(QWidget *parent);
	~ScriptEditWidget();

	bool eventFilter(QObject *obj, QEvent *ev);

public:
	void setScriptType(const QString &name);

Q_SIGNALS:
	void onEditButtonClicked(QWidget*);

protected:
	void paintEvent(QPaintEvent *);

private Q_SLOTS:
	void buttonClicked();

private:
	QLabel *m_label;
	QToolButton *m_button;
};


static inline void setupTreeViewEditorMargin(QLayout *lt)
{
	enum { DecorationMargin = 4 };
	if (QApplication::layoutDirection() == Qt::LeftToRight)
		lt->setContentsMargins(DecorationMargin, 0, 0, 0);
	else
		lt->setContentsMargins(0, 0, DecorationMargin, 0);
}

ScriptEditWidget::ScriptEditWidget(QWidget *parent) :
	QWidget(parent),
	m_label(new QLabel),
	m_button(new QToolButton)
{
	QHBoxLayout *lt = new QHBoxLayout(this);
	setupTreeViewEditorMargin(lt);
	lt->setSpacing(0);
	lt->addWidget(m_label);
	lt->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored));

	m_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
	m_button->setFixedWidth(20);
	setFocusProxy(m_button);
	setFocusPolicy(m_button->focusPolicy());
	m_button->setText(tr("..."));
	m_button->installEventFilter(this);
	connect(m_button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
	lt->addWidget(m_button);
	m_label->setText("Script");
}

ScriptEditWidget::~ScriptEditWidget()
{
	
}

void ScriptEditWidget::setScriptType(const QString &type)
{
	m_label->setText(type);
}

void ScriptEditWidget::buttonClicked()
{
	emit onEditButtonClicked(this);
}

bool ScriptEditWidget::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == m_button) {
		switch (ev->type()) {
		case QEvent::KeyPress:
		case QEvent::KeyRelease: { // Prevent the QToolButton from handling Enter/Escape meant control the delegate
			switch (static_cast<const QKeyEvent*>(ev)->key()) {
			case Qt::Key_Escape:
			case Qt::Key_Enter:
			case Qt::Key_Return:
				ev->ignore();
				return true;
			default:
				break;
			}
		}
								 break;
		default:
			break;
		}
	}
	return QWidget::eventFilter(obj, ev);
}

void ScriptEditWidget::paintEvent(QPaintEvent *)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

class ScriptEditorFactoryPrivate 
{
	ScriptEditorFactory *q_ptr;
	Q_DECLARE_PUBLIC(ScriptEditorFactory)
public:
	typedef QList<ScriptEditWidget *> EditorList;
	typedef QMap<QtProperty *, EditorList> PropertyToEditorListMap;
	typedef QMap<ScriptEditWidget *, QtProperty *> EditorToPropertyMap;

	ScriptEditWidget *createEditor(QtProperty *property, QWidget *parent);
	void initializeEditor(QtProperty *property, ScriptEditWidget *e);
	void slotEditorDestroyed(QObject *object);

	PropertyToEditorListMap  m_createdEditors;
	EditorToPropertyMap m_editorToProperty;
};

ScriptEditWidget *ScriptEditorFactoryPrivate::createEditor(QtProperty *property, QWidget *parent)
{
	ScriptEditWidget *editor = new ScriptEditWidget(parent);
	initializeEditor(property, editor);
	return editor;
}

void ScriptEditorFactoryPrivate::initializeEditor(QtProperty *property, ScriptEditWidget *editor)
{
	typename PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
	if (it == m_createdEditors.end())
		it = m_createdEditors.insert(property, EditorList());
	it.value().append(editor);
	m_editorToProperty.insert(editor, property);
}

void ScriptEditorFactoryPrivate::slotEditorDestroyed(QObject *object)
{
	const typename EditorToPropertyMap::iterator ecend = m_editorToProperty.end();
	for (typename EditorToPropertyMap::iterator itEditor = m_editorToProperty.begin(); itEditor != ecend; ++itEditor) {
		if (itEditor.key() == object) {
			ScriptEditWidget *editor = itEditor.key();
			QtProperty *property = itEditor.value();
			const typename PropertyToEditorListMap::iterator pit = m_createdEditors.find(property);
			if (pit != m_createdEditors.end()) {
				pit.value().removeAll(editor);
				if (pit.value().empty())
					m_createdEditors.erase(pit);
			}
			m_editorToProperty.erase(itEditor);
			return;
		}
	}
}

ScriptEditorFactory::ScriptEditorFactory(QObject *parent)
	: QtAbstractEditorFactory<ScriptPropertyManager>(parent)
{
	d_ptr = new ScriptEditorFactoryPrivate();
	d_ptr->q_ptr = this;

}

ScriptEditorFactory::~ScriptEditorFactory()
{
	qDeleteAll(d_ptr->m_editorToProperty.keys());
	delete d_ptr;
}

void ScriptEditorFactory::connectPropertyManager(ScriptPropertyManager *manager)
{
}

QWidget *ScriptEditorFactory::createEditor(ScriptPropertyManager *manager,
	QtProperty *property, QWidget *parent)
{
	ScriptEditWidget *editor = d_ptr->createEditor(property, parent);

	editor->setScriptType(manager->value(property).type);

	connect(editor, SIGNAL(onEditButtonClicked(QWidget*)), this, SLOT(editButtonClicked(QWidget*)));
	connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));

	return editor;
}

void ScriptEditorFactory::editButtonClicked(QWidget* editor)
{
	ScriptEditorFactoryPrivate::EditorToPropertyMap::const_iterator it = d_ptr->m_editorToProperty.constFind((ScriptEditWidget*)editor);
	if (it == d_ptr->m_editorToProperty.end()) return;

	emit editButtonClicked(it.value());
}

void ScriptEditorFactory::slotEditorDestroyed(QObject* obj)
{
	d_ptr->slotEditorDestroyed(obj);
}

void ScriptEditorFactory::disconnectPropertyManager(ScriptPropertyManager *manager)
{

}

#include "AT4_ScriptVariant.moc"
