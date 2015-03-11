/***************************************************

				AXIA|Trace3

	(C) Copyright  Jean(www.thecodeway.com). 2013
***************************************************/

#include "AxTrace.h"
#if defined(_MSC_VER)
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <strsafe.h>
#pragma comment(lib, "ws2_32.lib")
#endif

/*---------------------------------------------------------------------------------------------*/
#define DEFAULT_AXTRACE_SERVER_IP		"127.0.0.1"
#define DEFAULT_AXTRACE_SERVER_PORT		(1978)
#define AXTRACE_MAX_TRACE_STRING_LENGTH	(2048)

#define AXTRACE_CMD_TYPE_TRACE		(1)
#define AXTRACE_CMD_TYPE_VALUE		(2)

/*---------------------------------------------------------------------------------------------*/
/* AxTrace Global data  */
typedef struct
{
	int		is_init_succ;			/* 0 means not, 1 means yes*/
	struct sockaddr_in address;		/* axtrace server address */
	SOCKET	sfd;					/* socket file desc*/
} axtrace_contex_s;

/*---------------------------------------------------------------------------------------------*/
/* axtrace communication data struct*/
typedef __declspec(align(1)) struct
{
	unsigned short	length;			/* length */
	unsigned char	flag;			/* magic flag, always 'A' */
	unsigned char	type;			/* command type AXTRACE_CMD_TYPE_* */
	unsigned int	pid;			/* process id*/
	unsigned int	tid;			/* thread id*/
} axtrace_head_s;

/* axtrace trace data struct*/
typedef __declspec(align(1)) struct
{
	axtrace_head_s	head;			/* common head */
	unsigned short	code_page;		/* code page */
	unsigned short	length;			/* trace string length */
									/* trace string data with '\0' ended */
} axtrace_trace_s;

/*---------------------------------------------------------------------------------------------*/
static axtrace_contex_s* _axtrace_try_init(const char* server_ip, unsigned short server_port)
{
	axtrace_contex_s* ctx = (axtrace_contex_s*)LocalAlloc(LPTR, sizeof(axtrace_contex_s));
	if (ctx == 0)
	{
		//TODO: fatal error, should stop the process
		return 0;
	}
	memset(ctx, 0, sizeof (axtrace_contex_s));

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 1), &wsadata);

	ctx->address.sin_family = AF_INET;
	ctx->address.sin_port = htons(server_port != 0 ? server_port : DEFAULT_AXTRACE_SERVER_PORT);
	if (0 == InetPtonA(AF_INET,
		server_ip ? server_ip : DEFAULT_AXTRACE_SERVER_IP,
		&(ctx->address.sin_addr)))
	{
		return ctx;
	}

	/* connect to axtrace server*/
	ctx->sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(ctx->sfd, (const struct sockaddr*)&(ctx->address), sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		closesocket(ctx->sfd);
		return ctx;
	}

	/* init success */
	ctx->is_init_succ = 1;
	return ctx;
}

/*---------------------------------------------------------------------------------------------*/
static axtrace_contex_s* _axtrace_get_thread_contex(const char* server_ip, unsigned short server_port)
{
	static __declspec(thread) axtrace_contex_s* s_the_thread_data = 0;

	if (s_the_thread_data != 0) {
		/* already try init in this thread */
		return (s_the_thread_data->is_init_succ != 0) ? s_the_thread_data : 0;
	}

	/* try init */
	return s_the_thread_data = _axtrace_try_init(server_ip, server_port);
}

/*---------------------------------------------------------------------------------------------*/
void axtrace(unsigned int type, const char *format, ...)
{
	/* buf for send , call send() once*/
	axtrace_contex_s* ctx;
	va_list ptr = 0;
	HRESULT hr;
	size_t contents_byte_size, final_length;
	int send_len;

	char buf[sizeof(axtrace_trace_s)+AXTRACE_MAX_TRACE_STRING_LENGTH] = { 0 };
	axtrace_trace_s* trace_head = (axtrace_trace_s*)(buf);
	char* trace_string = (char*)(buf + sizeof(axtrace_trace_s));

	/* is init ok? */
	ctx = _axtrace_get_thread_contex(0, 0);
	if (ctx == 0) return;

	/* Create String Contents*/
	va_start(ptr, format);
	hr = StringCbVPrintfA(trace_string, AXTRACE_MAX_TRACE_STRING_LENGTH, format, ptr);
	va_end(ptr);
	/* failed ?*/
	if (FAILED(hr)) return;

	/** get string length*/
	hr = StringCbLengthA(trace_string, AXTRACE_MAX_TRACE_STRING_LENGTH - 1, &contents_byte_size);
	/* failed ?*/
	if (FAILED(hr)) return;

	/* add '\0' ended */
	contents_byte_size += 1;	

	/* fill the trace head data */
	final_length = sizeof(axtrace_trace_s)+contents_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_TRACE;
	trace_head->head.pid = GetCurrentProcessId();
	trace_head->head.tid = GetCurrentThreadId();

	trace_head->code_page = 0;	/* TODO: get current system code page*/
	trace_head->length = (unsigned short)contents_byte_size;

	/* send to axtrace server*/
	send_len = send(ctx->sfd, buf, (int)final_length, MSG_DONTROUTE);

	/*TODO: check result, may be reconnect to server */
	return;
}