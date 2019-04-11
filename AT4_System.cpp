#include "stdafx.h"
#include "AT4_System.h"
#include "AT4_MainWindow.h"
#include "AT4_Config.h"
#include "AT4_Incoming.h"
#include "AT4_MessageQueue.h"
#include "AT4_Filter.h"
#include "AT4_Scene2D.h"
#include "AT4_Map2DChild.h"

//--------------------------------------------------------------------------------------------
System* System::s_singleton = nullptr;

//--------------------------------------------------------------------------------------------
System::System()
	: m_theApplication(nullptr)
	, m_config(nullptr)
	, m_incoming(nullptr)
	, m_mainWindow(nullptr)
	, m_messageQueue(nullptr)
	, m_filter(nullptr)
{
	s_singleton = this;
}

//--------------------------------------------------------------------------------------------
System::~System()
{
	Map2DChild::deleteCachedObject();
	LogMessage::deletePool();
	ValueMessage::deletePool();
	Begin2DSceneMessage::deletePool();
	Update2DActorMessage::deletePool();
	End2DSceneMessage::deletePool();

	delete m_filter;
	delete m_messageQueue;
	delete m_mainWindow;
	delete m_incoming;
	delete m_config;
	delete m_theApplication;
}

//--------------------------------------------------------------------------------------------
bool System::init(int argc, char *argv[])
{
	m_theApplication = new QApplication(argc, argv);
	QCoreApplication::setApplicationName("AxTrace4");
	QCoreApplication::setOrganizationName("thecodeway.com");
	QCoreApplication::setApplicationVersion("4.0");
	m_theApplication->setWindowIcon(QIcon(":/images/AxTrace.ico"));

	QCommandLineParser parser;
	parser.setApplicationDescription("AxTrace");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.process(*m_theApplication);

	//allocate managers
	m_config = new Config();
	m_incoming = new Incoming();
	m_messageQueue = new MessageQueue();
	m_filter = new Filter();

	Map2DChild::initCachedObject();

	//init managers
	m_config->loadSetting();
	if (!(m_filter->init(m_config))) {
		return false;
	}

	if (!(m_incoming->init())) {
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------------
int System::run(void)
{
	m_mainWindow = new MainWindow();
	m_mainWindow->show();

	//enter loop...
	int retCode = m_theApplication->exec();

	m_config->saveSetting();
	m_incoming->closeListen();

	return retCode;
}