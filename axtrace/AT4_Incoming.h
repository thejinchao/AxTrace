/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

class Incoming
{
public:
	//default listen port is 1978 :)
	enum { DEFAULT_PORT = 1978 };

	/** create receive thread */
	bool init(void);
	/** try close receive thread*/
	void closeListen(void);
	//kick out connection
	void kickConnection(cyclone::ConnectionPtr conn);

private:
	cyclone::TcpServer* m_server;

	void on_connected(cyclone::TcpServer* server, int32_t thread_index, cyclone::ConnectionPtr conn);
	void on_message(cyclone::TcpServer* server, int32_t thread_index, cyclone::ConnectionPtr conn);
	void on_close(cyclone::TcpServer* server, int32_t thread_index, cyclone::ConnectionPtr conn);

public:
	Incoming();
	virtual ~Incoming();
};
