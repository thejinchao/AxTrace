/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

#include <QMainWindow>
#include <QEvent>
#include "AT4_ChildInterface.h"

class IChild;
class LogChild;
class ValueChild;
class Map2DChild;
class Message;
class ShakehandMessage;
class LogMessage;
class ValueMessage;
class Begin2DSceneMessage;
class Update2DActorMessage;
class End2DSceneMessage;
class Add2DActorLogMessage;

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QCustomEvent;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public:

	class AxTraceEvent : public QEvent
	{
	public:
		enum { Type = QEvent::User + 100 };

		AxTraceEvent() : QEvent((QEvent::Type)Type) {}
		~AxTraceEvent() {}
	};

public:
	LogChild* getLogChild(const QString& title);
	ValueChild* getValueChild(const QString& title);
	Map2DChild* getMap2DChild(const QString& title);

	void notifySelectionChanged(void) 
	{
		updateMenus();
	}

	void notifySubWindowClose(IChild::Type t, const QString& title);

	qint32 getLogChildCounts(void) const {
		return m_logChildMap.size();
	}
private:
	void _processAxTraceData(Message* msg);
	void _onShakeHand(ShakehandMessage* msg);
	void _insertLog(LogMessage* msg);
	void _insertValue(ValueMessage* msg);
	void _begin2DScene(Begin2DSceneMessage* msg);
	void _update2DActor(Update2DActorMessage* msg);
	void _end2DScene(End2DSceneMessage* msg);
	void _addActor2DLog(Add2DActorLogMessage* msg);

private:
	typedef QMap<QString, LogChild*> LogChildMap;
	LogChildMap m_logChildMap;

	typedef QMap<QString, ValueChild*> ValueChildMap;
	ValueChildMap m_valueChildMap;

	typedef QMap<QString, Map2DChild*> Map2DChildMap;
	Map2DChildMap m_map2dChildMap;

private slots:
    void _onSaveAs();
	void _onCapture();
	void _onDocumentPause();
	void _onAutoScroll();
	void _onShowGrid();
    void _onCopy();
	void _onClean();
	void _onCleanAll();
	void _onSetting();
	void _onAbout();

    void updateMenus();
    void updateWindowMenu();

private:
	void closeEvent(QCloseEvent *event) override;
	bool event(QEvent* e) override;

	void createActions();
    void createStatusBar();

    void _restoreSettings(void);

	IChild *activeMdiChild() const;

    QMdiArea *m_mdiArea;

    QMenu *m_windowMenu;

	QAction* m_saveAsAct;
	QAction* m_captureAct;
	QAction* m_docPauseAct;

	QAction* m_autoScrollAct;
	QAction* m_showGridAct;
	QAction* m_copyAct;
	QAction* m_cleanAct;
	QAction* m_cleanAllAct;

	QAction* m_settingAct;
	QAction* m_aboutAct;

    QAction *m_closeAct;
    QAction *m_closeAllAct;
    QAction *m_tileAct;
    QAction *m_cascadeAct;
    QAction *m_nextAct;
    QAction *m_previousAct;

    QAction *m_windowMenuSeparatorAct;
};
