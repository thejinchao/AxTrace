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

	virtual void on_connection_callback(cyclone::TcpServer* server, int32_t thread_index, cyclone::Connection* conn);
	virtual void on_message_callback(cyclone::TcpServer* server, int32_t thread_index, cyclone::Connection* conn);
	virtual void on_close_callback(cyclone::TcpServer* server, int32_t thread_index, cyclone::Connection* conn);
	virtual void on_extra_workthread_msg(cyclone::TcpServer* server, int32_t thread_index, cyclone::Packet* msg);

public:
	Incoming();
	virtual ~Incoming();
};

}
