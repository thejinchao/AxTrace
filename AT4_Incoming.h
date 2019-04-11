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

private:
	cyclone::TcpServer* m_server;

	virtual void on_message(cyclone::TcpServer* server, int32_t thread_index, cyclone::ConnectionPtr conn);
public:
	Incoming();
	virtual ~Incoming();
};
