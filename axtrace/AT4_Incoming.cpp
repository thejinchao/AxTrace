/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"

#include "AT4_Incoming.h"
#include "AT4_Interface.h"
#include "AT4_System.h"
#include "AT4_Config.h"
#include "AT4_MessageQueue.h"
#include "AT4_Session.h"

//--------------------------------------------------------------------------------------------
Incoming::Incoming()
	: m_server(nullptr)
{
	//set cyclone log level
	cyclone::set_log_threshold(cyclone::L_ERROR);
}

//--------------------------------------------------------------------------------------------
Incoming::~Incoming()
{
	close();
}

//--------------------------------------------------------------------------------------------
bool Incoming::init(qint32 listenPort)
{
	Q_ASSERT(m_server == nullptr);

	//begin listen
	cyclone::Address address(listenPort, false);
	m_server = new cyclone::TcpServer;
	m_server->m_listener.on_connected = std::bind(&Incoming::on_connected, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_server->m_listener.on_message = std::bind(&Incoming::on_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	m_server->m_listener.on_close = std::bind(&Incoming::on_close, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	if (!(m_server->bind(address, false))) return false;
	return m_server->start(cyclone::sys_api::get_cpu_counts());
}

//--------------------------------------------------------------------------------------------
void Incoming::kickConnection(cyclone::TcpConnectionPtr conn)
{
	m_server->shutdown_connection(conn);
}

//--------------------------------------------------------------------------------------------
void Incoming::on_connected(cyclone::TcpServer* server, int32_t thread_index, cyclone::TcpConnectionPtr conn)
{
	SessionManager* sessionManager = System::getSingleton()->getSessionManager();

	sessionManager->onSessionConnected(conn);
}

//--------------------------------------------------------------------------------------------
void Incoming::on_close(cyclone::TcpServer* server, int32_t thread_index, cyclone::TcpConnectionPtr conn)
{
	SessionManager* sessionManager = System::getSingleton()->getSessionManager();

	sessionManager->onSessionClose(conn);
}

//--------------------------------------------------------------------------------------------
void Incoming::on_message(cyclone::TcpServer* server, int32_t thread_index, cyclone::TcpConnectionPtr conn)
{
	cyclone::RingBuf& input_buf = conn->get_input_buf();

	if (System::getSingleton()->getConfig()->getCapture())
	{
		QDateTime timeNow = QDateTime::currentDateTime();

		cyclone::RingBuf& input = conn->get_input_buf();

		//peek size, and check valid
		do {
			size_t input_size = input.size();
			if (input_size < sizeof(axtrace_head_s)) return;

			axtrace_head_s head;
			if (sizeof(head) != input.peek(0, &head, sizeof(head)) || head.flag != 'A') {
				//error!, kick off this session
				m_server->shutdown_connection(conn);
				return;
			}

			//message type or message size is not valid
			qint32 maxMessageSize = Message::getMessageMaxSize(head.type);
			if (maxMessageSize<0 || head.length>maxMessageSize) {
				//error!, kick off this session
				m_server->shutdown_connection(conn);
				return;
			}

			//package is not completed yet
			if (input_size < head.length) {
				return;
			}

			System::getSingleton()->getMessageQueue()->insertMessage(&input, head.length, timeNow.time(), conn->get_id());
		} while (true);
	}

}

//--------------------------------------------------------------------------------------------
void Incoming::close(void)
{
	if (m_server == nullptr) return;

	//wait cyclone quit...
	m_server->stop();
	m_server->join();

	delete m_server;
	m_server = nullptr;
}

