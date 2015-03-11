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
	: m_nListenPort(DEFAULT_PORT)	
	, m_hReceiveThread(0)
	, m_opPull(0)
	, m_opQuit(0)
	, QUIT_SIGNAL_PORT("inproc://quit-signal")
{
}

//--------------------------------------------------------------------------------------------
Incoming::~Incoming()
{
}

//--------------------------------------------------------------------------------------------
bool Incoming::init_cyclone(void)
{
	cyclone::Address address(DEFAULT_PORT, false);
	m_server = new cyclone::TcpServer(address, this);

	m_server->set_message_callback(_cyclone_message_callback_entry);

	m_server->start(2);

	return true;
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
void Incoming::closeListen_cyclone(void)
{
	delete m_server;
}

//--------------------------------------------------------------------------------------------
bool Incoming::init(void)
{
	return init_cyclone();
	assert(m_hReceiveThread==0);
	const Config* config = System::getSingleton()->getConfig();

	//create pull port
	if (!_createPullPort()) return false;

	//create quit op port
	m_opQuit = zmq_socket(System::getSingleton()->getZeroMQ(), ZMQ_PULL);
	if (0 != zmq_bind(m_opQuit, QUIT_SIGNAL_PORT.c_str())) return false;

	//begin listen thread
	unsigned int threadID;
	m_hReceiveThread = (HANDLE)::_beginthreadex(0, 0, 
		__threadEntry, this, THREAD_QUERY_INFORMATION, &threadID);
	if(m_hReceiveThread == 0) 
	{
		zmq_close(m_opPull);
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------------
unsigned int Incoming::__threadEntry(void* param)
{
	return ((Incoming*)param)->_threadEntry();
}

//--------------------------------------------------------------------------------------------
bool Incoming::_createPullPort(void)
{
	void* ctx = System::getSingleton()->getZeroMQ();

	//try create zmp port
	void* port = zmq_socket(System::getSingleton()->getZeroMQ(), ZMQ_PULL);

	char temp[MAX_PATH]={0};
	StringCchPrintfA(temp, MAX_PATH, "tcp://*:%d", m_nListenPort);
	if(0==zmq_bind(port, temp))
	{
		m_opPull = port;
		m_strListenPort = temp;
		return true;
	}
	zmq_close(port);
	return false;
}

//--------------------------------------------------------------------------------------------
unsigned int Incoming::_threadEntry(void)
{
	//work loop...
	while(true)
	{
		zmq_msg_t recv_command;
		zmq_msg_init(&recv_command);

		//receive
		zmq_pollitem_t wait_items[] = {
			{ m_opQuit, 0, ZMQ_POLLIN, 0 },
			{ m_opPull, 0, ZMQ_POLLIN, 0 }
		};

		//wait receive command or quit signal
		zmq_poll(wait_items, 2, -1);

		//receive quit signal, quit!
		if ((wait_items[0].revents & ZMQ_POLLIN)) break;

		//receive message
		if ((wait_items[1].revents & ZMQ_POLLIN) && zmq_msg_recv(&recv_command, m_opPull, 0)>0)
		{
			if(System::getSingleton()->getConfig()->getCapture())
			{
				SYSTEMTIME tTime;
				GetLocalTime(&tTime);

				System::getSingleton()->getMessageQueue()->insertMessage((const char*)zmq_msg_data(&recv_command), zmq_msg_size(&recv_command), &tTime);
			}
		}
		zmq_msg_close(&recv_command);
	}

	_endthread();
	return 0;
}

//--------------------------------------------------------------------------------------------
void Incoming::closeListen(void)
{
	//push a dummy msg to let incoming loop quit
	void* s = zmq_socket(System::getSingleton()->getZeroMQ(), ZMQ_PUSH);
	zmq_connect(s, QUIT_SIGNAL_PORT.c_str());
	zmq_send(s, " ", 1, 0);
	zmq_close(s);

	//wait thred quit(max 1sec)
	if(WAIT_TIMEOUT == WaitForSingleObject(m_hReceiveThread, 1000))
	{
		//destroy thread
		::TerminateThread(m_hReceiveThread, 0);
	}

	//close zmq bind
	zmq_unbind(m_opPull, m_strListenPort.c_str());

	zmq_close(m_opPull);
	zmq_close(m_opQuit);
}

}

