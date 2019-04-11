#include "stdafx.h"

#include "AT4_Incoming.h"
#include "AT4_Interface.h"
#include "AT4_System.h"
#include "AT4_Config.h"
#include "AT4_MessageQueue.h"

//--------------------------------------------------------------------------------------------
Incoming::Incoming()
	: m_server(nullptr)
{
}

//--------------------------------------------------------------------------------------------
Incoming::~Incoming()
{
	if (m_server) {
		delete m_server;
	}
}

//--------------------------------------------------------------------------------------------
bool Incoming::init(void)
{
	//disable cyclone log
	cyclone::set_log_threshold(cyclone::L_MAXIMUM_LEVEL);

	cyclone::Address address(DEFAULT_PORT, false);
	m_server = new cyclone::TcpServer("axtrace", nullptr);
	m_server->m_listener.onMessage = std::bind(&Incoming::on_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	if (!(m_server->bind(address, false))) return false;
	return m_server->start(cyclone::sys_api::get_cpu_counts());
}

//--------------------------------------------------------------------------------------------
void Incoming::on_message(cyclone::TcpServer* server, int32_t thread_index, cyclone::ConnectionPtr conn)
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

			//package is completed
			if (input_size < head.length) {
				//TODO: size too large?
				return;
			}

			System::getSingleton()->getMessageQueue()->insertMessage(&input, head.length, timeNow.time());
		} while (true);
	}

}

//--------------------------------------------------------------------------------------------
void Incoming::closeListen(void)
{
	//wait cyclone quit...
	m_server->stop();
	m_server->join();
}

