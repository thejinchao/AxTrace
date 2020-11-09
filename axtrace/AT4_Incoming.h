/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#pragma once

class Incoming
{
public:
	// create receive thread and start listen port
	bool init(qint32 listenPort);
	// try close current network thread
	void close(void);
	//kick out connection
	void kickConnection(cyclone::TcpConnectionPtr conn);

private:
	cyclone::TcpServer* m_server;

	void on_connected(cyclone::TcpServer* server, int32_t thread_index, cyclone::TcpConnectionPtr conn);
	void on_message(cyclone::TcpServer* server, int32_t thread_index, cyclone::TcpConnectionPtr conn);
	void on_close(cyclone::TcpServer* server, int32_t thread_index, cyclone::TcpConnectionPtr conn);

public:
	Incoming();
	virtual ~Incoming();
};
