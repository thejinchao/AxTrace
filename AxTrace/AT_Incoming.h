/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once

namespace AT3
{

/** Incoming module
*/
class Incoming
{
public:
	/** create receive thread */
	bool init(void);
	/** try close receive thread*/
	void closeListen(void);

private:
	HANDLE		m_hReceiveThread;	//!< receive thread handle
	int			m_nListenPort;		//!< tcp port
	std::string	m_strListenPort;	//!< listen port(string format)

	//default listen port is 1978 :)
	enum { DEFAULT_PORT=1978 };
	void*		m_opPull;			//!< zmp port to recive msg

	//force quit signal
	void*		m_opQuit;
	
	const std::string QUIT_SIGNAL_PORT;

private:
	/** thread entry function */
	static unsigned int __stdcall __threadEntry(void* param);
	/** thread function	*/
	unsigned int _threadEntry(void);
	/** create zmp port*/
	bool _createPullPort(void);

public:
	Incoming();
	virtual ~Incoming();
};

}
