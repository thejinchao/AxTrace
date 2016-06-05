/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once

namespace AT3
{
/** pre-define class
*/
class Config;
class MainFrame;
class Incoming;
class MessageQueue;
class Message;
class LogMessage;
class ValueMessage;
class G2DCleanMapMessage;
class Filter;

/**Global System
*/
class System : public CIdleHandler
{
	/*************************************************************************
		Public Methods
	*************************************************************************/
public:
	/** Get singleton
	*/
	static System* getSingleton(void) { return s_pMe; };
	/** System init
	*/
	bool init(HINSTANCE hInstance, LPSTR lpCmdLine);
	/** System run
	*/
	void run(void);
	/** System shut down
	*/
	void release(void);

	/** Get main frame */
	MainFrame* getMainFrame(void) { return m_wndMainFrame; }
	/** Get Message Looper*/
	CMessageLoop* getMessageLoop(void) { return &m_theMsgLoop; }
	/** Get Message Queue */
	MessageQueue* getMessageQueue(void) { return m_msgQueue; }
	/** get config*/
	Config* getConfig(void) { return m_theConfig; }
	/** get app module */
	CAppModule& getAppModule(void) { return m_AppModule; }
	/** get filter */
	Filter* getFilter(void) { return m_filter; }

	/*************************************************************************
			Inherit Methods
	*************************************************************************/
public:
	/** Interface for WTL idle processing
	*/
	virtual BOOL OnIdle(void);

private:
	/** process Axtrace data node */
	void _processAxTraceData(const Message* message);
	/** insert string log*/
	void _insertStringLog(const LogMessage* message);
	/** insert string log*/
	void _watchValue(const ValueMessage* message);
	/** insert 2d map*/
	void _2DCleanMap(const G2DCleanMapMessage* message);

	/*************************************************************************
		Implementation Data
	*************************************************************************/
private:
	static System*			s_pMe;

	HANDLE					m_hMutex;			//!< Global mutex
	HINSTANCE				m_hInstance;		//!< App instance
	std::string				m_strCmdLine;		//!< Command line
	CAppModule				m_AppModule;		//!< Global WTL Application Module
	CMessageLoop			m_theMsgLoop;		//!< WTL Message looper
	Config*					m_theConfig;		//!< Global config system
	MainFrame*				m_wndMainFrame;		//!< Main MDI Frame
	Incoming*				m_pIncoming;		//!< Incoming thread to receive msg
	MessageQueue*			m_msgQueue;			//!< Message Queue
	Filter*					m_filter;			//!< Display filter

	/*************************************************************************
		Construction and Destruction
	*************************************************************************/
public:
	System();
	~System();

};

}