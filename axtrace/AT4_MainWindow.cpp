/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"

#include "AT4_MainWindow.h"
#include "AT4_LogChild.h"
#include "AT4_ValueChild.h"
#include "AT4_System.h"
#include "AT4_MessageQueue.h"
#include "AT4_Message.h"
#include "AT4_Filter.h"
#include "AT4_ChildInterface.h"
#include "AT4_Config.h"
#include "AT4_Map2DChild.h"
#include "AT4_Scene2D.h"
#include "AT4_SettingDialog.h"
#include "AT4_Session.h"

//--------------------------------------------------------------------------------------------
MainWindow::MainWindow()
    : m_mdiArea(new QMdiArea)
{
	m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    setCentralWidget(m_mdiArea);
    connect(m_mdiArea, &QMdiArea::subWindowActivated,
            this, &MainWindow::updateMenus);

    createActions();
    createStatusBar();
    updateMenus();

    _restoreSettings();

    setWindowTitle(tr("AxTrace"));

    setUnifiedTitleAndToolBarOnMac(true);
}

//--------------------------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *event)
{
    m_mdiArea->closeAllSubWindows();
    if (m_mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
		System::getSingleton()->getConfig()->setMainGeometry(saveGeometry());
        event->accept();
    }
}

//--------------------------------------------------------------------------------------------
bool MainWindow::event(QEvent* e)
{
	if (e->type() == AxTraceEvent::Type)
	{
		MessageVector msgVector;
		System::getSingleton()->getMessageQueue()->popMessage(msgVector);

		if (!msgVector.empty())
		{
			for (auto msg : msgVector) 
			{
				_processAxTraceData(msg);
				msg->reccycleMessage();
			}
			msgVector.clear();
		}
	}
	return QMainWindow::event(e);
}

//--------------------------------------------------------------------------------------------
void MainWindow::_processAxTraceData(Message* msg)
{
	switch (msg->getType())
	{
	case AXTRACE_CMD_TYPE_SHAKEHAND:
		_onShakeHand((ShakehandMessage*)msg);
		break;

	case AXTRACE_CMD_TYPE_LOG:
		_insertLog((LogMessage*)msg);
		break;

	case AXTRACE_CMD_TYPE_VALUE:
		_insertValue((ValueMessage*)msg);
		break;

	case AXTRACE_CMD_TYPE_2D_BEGIN_SCENE:
		_begin2DScene((Begin2DSceneMessage*)msg);
		break;

	case AXTRACE_CMD_TYPE_2D_ACTOR:
		_update2DActor((Update2DActorMessage*)msg);
		break;

	case AXTRACE_CMD_TYPE_2D_END_SCENE:
		_end2DScene((End2DSceneMessage*)msg);
		break;

	case AXTRACE_CMD_TYPE_2D_ACTOR_LOG:
		_addActor2DLog((Add2DActorLogMessage*)msg);
		break;

	default: break;
	}
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onShakeHand(ShakehandMessage* msg)
{

}

//--------------------------------------------------------------------------------------------
void MainWindow::_insertLog(LogMessage* msg)
{
	Filter::ListResult filterResult;
	System::getSingleton()->getFilter()->onLogMessage(msg, filterResult);
	if (!filterResult.display) return;

	LogChild* child = getLogChild(filterResult.wndTitle);
	assert(child);

	child->insertLog(msg, filterResult);
}

//--------------------------------------------------------------------------------------------
void MainWindow::_insertValue(ValueMessage* msg)
{
	Filter::ListResult filterResult;
	System::getSingleton()->getFilter()->onValueMessage(msg, filterResult);
	if (!filterResult.display) return;

	ValueChild* child = getValueChild(filterResult.wndTitle);
	child->insertValue(msg, filterResult);
}

//--------------------------------------------------------------------------------------------
void MainWindow::_begin2DScene(Begin2DSceneMessage* msg)
{
	Map2DChild* child = getMap2DChild(msg->getSceneName());
	child->beginScene(msg);
}

//--------------------------------------------------------------------------------------------
void MainWindow::_update2DActor(Update2DActorMessage* msg)
{
	Filter::Actor2DResult filterResult;
	System::getSingleton()->getFilter()->onActor2DMessage(msg, filterResult);
	if (!filterResult.display) return;

	Map2DChild* child = getMap2DChild(msg->getSceneName());
	child->updateActor(msg, filterResult);
}

//--------------------------------------------------------------------------------------------
void MainWindow::_end2DScene(End2DSceneMessage* msg)
{
	Map2DChild* child = getMap2DChild(msg->getSceneName());
	child->endScene(msg);
}

//--------------------------------------------------------------------------------------------
void MainWindow::_addActor2DLog(Add2DActorLogMessage* msg)
{
	Map2DChild* child = getMap2DChild(msg->getSceneName());
	child->addActorLog(msg);
}

//--------------------------------------------------------------------------------------------
LogChild* MainWindow::getLogChild(const QString& title)
{
	auto it = m_logChildMap.find(title);
	if (it != m_logChildMap.end()) return it.value();

	LogChild *child = new LogChild(title);
	QMdiSubWindow* parent = m_mdiArea->addSubWindow(child);

	parent->resize(m_mdiArea->size()/2);

	child->init();
	child->show();

	m_logChildMap.insert(title, child);
	return child;
}

//--------------------------------------------------------------------------------------------
ValueChild* MainWindow::getValueChild(const QString& title)
{
	auto it = m_valueChildMap.find(title);
	if (it != m_valueChildMap.end()) return it.value();

	ValueChild* child = new ValueChild(title);
	QMdiSubWindow* parent = m_mdiArea->addSubWindow(child);

	parent->resize(m_mdiArea->size() / 2);

	child->init();
	child->show();

	m_valueChildMap.insert(title, child);
	return child;
}

//--------------------------------------------------------------------------------------------
Map2DChild* MainWindow::getMap2DChild(const QString& title)
{
	auto it = m_map2dChildMap.find(title);
	if (it != m_map2dChildMap.end()) return it.value();

	Map2DChild* child = new Map2DChild(title);

	QMdiSubWindow* parent = m_mdiArea->addSubWindow(child);
	parent->resize(m_mdiArea->size() / 2);

	child->init(parent);
	child->show();

	m_map2dChildMap.insert(title, child);
	return child;
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onSaveAs()
{
	IChild* activeChild = activeMdiChild();
	if (activeChild)
		activeChild->saveAs();
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onCapture()
{
	Config* config = System::getSingleton()->getConfig();
	config->setCapture(!(config->getCapture()));
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onDocumentPause()
{
	IChild* activeChild = activeMdiChild();
	if (activeChild)
		activeChild->switchPause();
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onAutoScroll()
{
	Config* config = System::getSingleton()->getConfig();
	config->setAutoScroll(!(config->getAutoScroll()));
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onShowGrid()
{
	Config* config = System::getSingleton()->getConfig();
	config->setShowGrid(!(config->getShowGrid()));

	auto windows = m_mdiArea->subWindowList();

	foreach(auto window, windows) {
		IChild *child = (IChild *)(window->widget()->userData(0));
		if (child && child->getType()== IChild::CT_2DMAP)
			child->update();
	}
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onCopy()
{
	IChild* activeChild = activeMdiChild();
	if (activeChild)
		activeChild->onCopy();
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onClean()
{
	IChild* activeChild = activeMdiChild();
	if (activeChild)
		activeChild->clean();
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onCleanAll()
{
	auto windows = m_mdiArea->subWindowList();

	foreach(auto window, windows) {
		IChild *child = (IChild *)(window->widget()->userData(0));
		if (child)
			child->clean();
	}
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onSetting()
{
	SettingDialog dialog(this);
	dialog.exec();
}

//--------------------------------------------------------------------------------------------
void MainWindow::_onAbout()
{
	QMessageBox msgBox;
	msgBox.setWindowTitle(tr("About AxTrace"));
	msgBox.setTextFormat(Qt::RichText); 
	msgBox.setText(tr("AXIA|Trace 4<br/>Copyright <a href=\"http://www.thecodeway.com\">www.thecodeway.com</a>"));
	msgBox.exec();
}

//--------------------------------------------------------------------------------------------
void MainWindow::notifySubWindowClose(IChild::Type t, const QString& title)
{
	switch (t)
	{
	case IChild::CT_LOG:
	{
		m_logChildMap.remove(title);
	}
		break;

	case IChild::CT_VALUE:
	{
		m_valueChildMap.remove(title);
	}
	break;

	case IChild::CT_2DMAP:
	{
		m_map2dChildMap.remove(title);
	}
	break;
	
	}
}

//--------------------------------------------------------------------------------------------
void MainWindow::updateMenus()
{
    bool hasMdiChild = (activeMdiChild() != 0);
	IChild* activeChild = activeMdiChild();
	Config* config = System::getSingleton()->getConfig();

	m_saveAsAct->setEnabled(hasMdiChild);
	m_captureAct->setChecked(config->getCapture());

	m_docPauseAct->setEnabled(hasMdiChild);
	m_docPauseAct->setChecked(activeChild && activeChild->isPause());

	m_autoScrollAct->setEnabled(activeChild && activeChild->getType() == IChild::CT_LOG);
	m_autoScrollAct->setChecked(config->getAutoScroll());

	m_showGridAct->setEnabled(activeChild && activeChild->getType() == IChild::CT_2DMAP);
	m_showGridAct->setChecked(config->getShowGrid());

	m_copyAct->setEnabled(activeChild && activeChild->copyAble());
	m_cleanAct->setEnabled(hasMdiChild);
	m_cleanAllAct->setEnabled(hasMdiChild);

	m_closeAct->setEnabled(hasMdiChild);
    m_closeAllAct->setEnabled(hasMdiChild);
	m_tileAct->setEnabled(hasMdiChild);
	m_cascadeAct->setEnabled(hasMdiChild);
	m_nextAct->setEnabled(hasMdiChild);
	m_previousAct->setEnabled(hasMdiChild);
	m_windowMenuSeparatorAct->setVisible(hasMdiChild);
}

//--------------------------------------------------------------------------------------------
void MainWindow::updateWindowMenu()
{
    m_windowMenu->clear();
    m_windowMenu->addAction(m_closeAct);
    m_windowMenu->addAction(m_closeAllAct);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_tileAct);
    m_windowMenu->addAction(m_cascadeAct);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_nextAct);
    m_windowMenu->addAction(m_previousAct);
    m_windowMenu->addAction(m_windowMenuSeparatorAct);

    QList<QMdiSubWindow *> windows = m_mdiArea->subWindowList();
	m_windowMenuSeparatorAct->setVisible(!windows.isEmpty());
	
    for (int i = 0; i < windows.size(); ++i) {
        QMdiSubWindow *mdiSubWindow = windows.at(i);
		IChild *child = (IChild *)(mdiSubWindow->widget()->userData(0));

		QString text = child->getTitle();

		QAction *action = m_windowMenu->addAction(text, mdiSubWindow, [this, mdiSubWindow]() {
            m_mdiArea->setActiveSubWindow(mdiSubWindow);
        });
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
    }
}

//--------------------------------------------------------------------------------------------
void MainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as", QIcon(":/images/save.png"));
    m_saveAsAct = new QAction(saveAsIcon, tr("Save &As..."), this);
    m_saveAsAct->setShortcuts(QKeySequence::SaveAs);
    m_saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(m_saveAsAct, &QAction::triggered, this, &MainWindow::_onSaveAs);
    fileMenu->addAction(m_saveAsAct);

	const QIcon captureIcon = QIcon(":/images/capture.png");
	m_captureAct = new QAction(captureIcon, tr("&Capture"), this);
	m_captureAct->setStatusTip(tr("Start/Stop capture"));
	m_captureAct->setCheckable(true);
	connect(m_captureAct, &QAction::triggered, this, &MainWindow::_onCapture);
	fileMenu->addAction(m_captureAct);

	const QIcon documentPauseIcon = QIcon(":/images/doc-pause.png");
	m_docPauseAct = new QAction(documentPauseIcon, tr("&Pause"), this);
	m_docPauseAct->setStatusTip(tr("Pause/Resume document capture"));
	m_docPauseAct->setCheckable(true);
	connect(m_docPauseAct, &QAction::triggered, this, &MainWindow::_onDocumentPause);
	fileMenu->addAction(m_docPauseAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), qApp, &QApplication::closeAllWindows);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAct);
	

	//----------------------------
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

	const QIcon autoScrollIcon = QIcon(":/images/auto-scroll.png");
	m_autoScrollAct = new QAction(autoScrollIcon, tr("Auto &Scroll"), this);
	m_autoScrollAct->setStatusTip(tr("Auto scroll to latest log"));
	m_autoScrollAct->setCheckable(true);
	connect(m_autoScrollAct, &QAction::triggered, this, &MainWindow::_onAutoScroll);
	editMenu->addAction(m_autoScrollAct);

	const QIcon showGridIcon = QIcon(":/images/grid.png");
	m_showGridAct = new QAction(showGridIcon, tr("Show &Grid"), this);
	m_showGridAct->setStatusTip(tr("Show/Hide scene grid"));
	m_showGridAct->setCheckable(true);
	connect(m_showGridAct, &QAction::triggered, this, &MainWindow::_onShowGrid);
	editMenu->addAction(m_showGridAct);


    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    m_copyAct = new QAction(copyIcon, tr("&Copy"), this);
    m_copyAct->setShortcuts(QKeySequence::Copy);
    m_copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(m_copyAct, &QAction::triggered, this, &MainWindow::_onCopy);
    editMenu->addAction(m_copyAct);

	const QIcon cleanIcon = QIcon(":/images/clean.png");
	m_cleanAct = new QAction(cleanIcon, tr("C&lean"), this);
	m_cleanAct->setStatusTip(tr("Clean current window"));
	connect(m_cleanAct, &QAction::triggered, this, &MainWindow::_onClean);
	editMenu->addAction(m_cleanAct);

	const QIcon cleanAllIcon = QIcon(":/images/clean-all.png");
	m_cleanAllAct = new QAction(cleanAllIcon, tr("Clean &All"), this);
	m_cleanAllAct->setStatusTip(tr("Clean all windows"));
	connect(m_cleanAllAct, &QAction::triggered, this, &MainWindow::_onCleanAll);
	editMenu->addAction(m_cleanAllAct);

	editMenu->addSeparator();

	const QIcon settingIcon = QIcon(":/images/setting.png");
	m_settingAct = new QAction(settingIcon, tr("S&etting..."), this);
	m_settingAct->setStatusTip(tr("Open setting window"));
	m_settingAct->setEnabled(true);
	connect(m_settingAct, &QAction::triggered, this, &MainWindow::_onSetting);
	editMenu->addAction(m_settingAct);

	//----------------------------

    m_windowMenu = menuBar()->addMenu(tr("&Window"));
    connect(m_windowMenu, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);

	const QIcon closeIcon = QIcon(":/images/close.png");
	m_closeAct = new QAction(closeIcon, tr("Cl&ose"), this);
	m_closeAct->setStatusTip(tr("Close the active window"));
    connect(m_closeAct, &QAction::triggered,
            m_mdiArea, &QMdiArea::closeActiveSubWindow);

	const QIcon closeAllIcon = QIcon(":/images/close-all.png");
	m_closeAllAct = new QAction(closeAllIcon, tr("Close &All"), this);
	m_closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(m_closeAllAct, &QAction::triggered, m_mdiArea, &QMdiArea::closeAllSubWindows);

	m_tileAct = new QAction(tr("&Tile"), this);
	m_tileAct->setStatusTip(tr("Tile the windows"));
    connect(m_tileAct, &QAction::triggered, m_mdiArea, &QMdiArea::tileSubWindows);

	m_cascadeAct = new QAction(tr("&Cascade"), this);
	m_cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(m_cascadeAct, &QAction::triggered, m_mdiArea, &QMdiArea::cascadeSubWindows);

	m_nextAct = new QAction(tr("Ne&xt"), this);
	m_nextAct->setShortcuts(QKeySequence::NextChild);
	m_nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(m_nextAct, &QAction::triggered, m_mdiArea, &QMdiArea::activateNextSubWindow);

	m_previousAct = new QAction(tr("Pre&vious"), this);
	m_previousAct->setShortcuts(QKeySequence::PreviousChild);
	m_previousAct->setStatusTip(tr("Move the focus to the previous window"));
    connect(m_previousAct, &QAction::triggered, m_mdiArea, &QMdiArea::activatePreviousSubWindow);

	m_windowMenuSeparatorAct = new QAction(this);
	m_windowMenuSeparatorAct->setSeparator(true);

    updateWindowMenu();

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction *aboutAct = helpMenu->addAction(QIcon(":/images/about.png"), tr("&About"), this, &MainWindow::_onAbout);
    aboutAct->setStatusTip(tr("Show the application's About box"));

	//Main Toolbar
	QToolBar *mainToolBar = addToolBar(tr("Main"));
	mainToolBar->addAction(m_saveAsAct);
	mainToolBar->addAction(m_captureAct);
	mainToolBar->addAction(m_docPauseAct);
	mainToolBar->addSeparator();
	mainToolBar->addAction(m_autoScrollAct);
	mainToolBar->addAction(m_showGridAct);
	mainToolBar->addAction(m_copyAct);
	mainToolBar->addAction(m_cleanAct);
	mainToolBar->addAction(m_cleanAllAct);
	mainToolBar->addAction(m_closeAllAct);
	mainToolBar->addSeparator();
	mainToolBar->addAction(m_settingAct);
}

//--------------------------------------------------------------------------------------------
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

//--------------------------------------------------------------------------------------------
void MainWindow::_restoreSettings(void)
{
	const QByteArray& geometry = System::getSingleton()->getConfig()->getMainGeometry();

    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 2, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

//--------------------------------------------------------------------------------------------
IChild *MainWindow::activeMdiChild() const
{
    if (QMdiSubWindow *activeSubWindow = m_mdiArea->activeSubWindow())
        return (IChild *)(activeSubWindow->widget()->userData(0));
    return nullptr;
}
