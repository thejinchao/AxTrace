/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/
#pragma once

namespace cyclone
{
	class TcpServer;
	class Connection;
}

namespace AT3
{

/** Incoming module
*/
class Incoming : public cyclone::TcpServer::Listener
{
public:
	//default listen port is 1978 :)
	enum { DEFAULT_PORT = 1978 };

	/** create receive thread */
	bool init(void);
	/** try close receive thread*/
	void closeListen(void);

private:
	cyclone::TcpServer* m_server;

	virtual void on_workthread_start(cyclone::TcpServer* server, int32_t thread_index, cyclone::Looper* looper);
	virtual void on_workthread_cmd(cyclone::TcpServer* server, int32_t thread_index, cyclone::Packet* cmd);

	virtual void on_connected(cyclone::TcpServer* server, int32_t thread_index, cyclone::ConnectionPtr conn);
	virtual void on_message(cyclone::TcpServer* server, int32_t thread_index, cyclone::ConnectionPtr conn);
	virtual void on_close(cyclone::TcpServer* server, int32_t thread_index, cyclone::ConnectionPtr conn);
public:
	Incoming();
	virtual ~Incoming();
};

}
