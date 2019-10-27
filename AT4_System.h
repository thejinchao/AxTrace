/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

class MainWindow;
class Incoming;
class Config;
class MessageQueue;
class Filter;
class SessionManager;

class System
{
public:
	bool init(int argc, char *argv[]);
	int run(void);

public:
	Config* getConfig(void) { return m_config; }
	MainWindow* getMainWindow(void) { return m_mainWindow; }
	MessageQueue* getMessageQueue(void) { return m_messageQueue; }
	Filter*	getFilter(void) { return m_filter; }
	SessionManager* getSessionManager(void) { return m_sessionManager; }
	Incoming* getIncoming(void) { return m_incoming; }

private:
	QApplication*	m_theApplication;
	Config*			m_config;
	Incoming*		m_incoming;
	MainWindow*		m_mainWindow;
	MessageQueue*	m_messageQueue;
	Filter*			m_filter;
	SessionManager* m_sessionManager;

public:
	System();
	~System();

	static System* getSingleton(void) { return s_singleton; };

private:
	static System*  s_singleton;
};
