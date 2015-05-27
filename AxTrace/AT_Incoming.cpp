/***************************************************

                     AXIA|Trace3		
				
                           (C) Copyright  Jean. 2013
***************************************************/

#include "StdAfx.h"
#include "AT_Incoming.h"
#include "AT_System.h"
#include "AT_MessageQueue.h"
#include "AT_Config.h"

namespace AT3
{

//--------------------------------------------------------------------------------------------
Incoming::Incoming()
	: m_server(0)
{
}

//--------------------------------------------------------------------------------------------
Incoming::~Incoming()
{
	if (m_server){
		delete m_server;
	}
}

//--------------------------------------------------------------------------------------------
bool Incoming::init(void)
{
	//disable cyclone log
	cyclone::set_log_threshold(cyclone::L_MAXIMUM_LEVEL);

	cyclone::Address address(DEFAULT_PORT, false);
	m_server = new cyclone::TcpServer(this);

	m_server->set_message_callback(_cyclone_message_callback_entry);

	m_server->bind(address, false);
	return m_server->start(2);
}

//--------------------------------------------------------------------------------------------
void Incoming::_cyclone_message_callback_entry(cyclone::TcpServer* server, cyclone::Connection* conn)
{
	((Incoming*)(server->get_callback_param()))->_cyclone_message_callback(conn);
}

//--------------------------------------------------------------------------------------------
void Incoming::_cyclone_message_callback(cyclone::Connection* conn)
{
	cyclone::RingBuf& input_buf = conn->get_input_buf();

	if (System::getSingleton()->getConfig()->getCapture())
	{
		SYSTEMTIME tTime;
		GetLocalTime(&tTime);

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

			System::getSingleton()->getMessageQueue()->insertMessage(&input, head.length, &tTime);
		} while (true);
	}

}

//--------------------------------------------------------------------------------------------
void Incoming::closeListen(void)
{
	//wait cyclone quit...
	m_server->stop();
}

}

