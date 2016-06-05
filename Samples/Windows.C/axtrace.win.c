/***************************************************

				AXIA|Trace3

	(C) Copyright  Jean(www.thecodeway.com). 2013
***************************************************/

#include "axtrace.win.h"

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
#define AXTRACE_MAX_TRACE_STRING_LENGTH	(0x8000)
#define AXTRACE_MAX_VALUENAME_LENGTH	(128)
#define AXTRACE_MAX_VALUE_LENGTH		(1024)
#define AXTRACE_MAX_MAP_NAME_LENGTH		(128)

#define AXTRACE_CMD_TYPE_LOG			(1)
#define AXTRACE_CMD_TYPE_VALUE			(2)
#define AXTRACE_CMD_TYPE_2D_CLEAN_MAP	(3)

/*---------------------------------------------------------------------------------------------*/
/* AxTrace Global data  */
typedef struct
{
	int		is_init_succ;			/* 0 means not, 1 means yes*/
	struct sockaddr_in address;		/* axtrace server address */
	SOCKET	sfd;					/* socket file desc*/
} axtrace_contex_s;

/*---------------------------------------------------------------------------------------------*/
#pragma pack(push)
#pragma pack(1)
/* axtrace communication data struct*/
typedef struct
{
	unsigned short	length;			/* length */
	unsigned char	flag;			/* magic flag, always 'A' */
	unsigned char	type;			/* command type AXTRACE_CMD_TYPE_* */
	unsigned int	pid;			/* process id*/
	unsigned int	tid;			/* thread id*/
	unsigned int	style;			/* trace style AXT_* */
} axtrace_head_s;

/* axtrace log data struct*/
typedef struct
{
	axtrace_head_s	head;			/* common head */
	unsigned short	code_page;		/* code page */
	unsigned short	length;			/* trace string length */

	/* [trace string data with '\0' ended] */
} axtrace_log_s;

typedef struct
{
	axtrace_head_s	head;			/* common head */
	unsigned int	value_type;		/* value type AXV_* */
	unsigned short	name_len;		/* length of value name */
	unsigned short	value_len;		/* length of value */

	/* [name buf  with '\0' ended]*/
	/* [value buf] */
} axtrace_value_s;


/*
            x_size
+----------------------------+
|                            |
|                            |
|                            |y_size
|                            |
|                            |
+----------------------------+
*/
typedef struct
{
	axtrace_head_s	head;			/* common head */
	double			x_size;			/* map size(x)*/
	double			y_size;			/* map size(y)*/
	unsigned short	name_len;		/* length of value name */

									/* [name buf  with '\0' ended]*/
									/* [value buf] */
} axtrace_2d_clean_map_s;


#pragma pack(pop)

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

	/* TODO: create non-blocking socket, so we can save some time when connect to server */

	/* set SO_LINGER off, make sure all data in send buf can be sended */
	struct linger linger_;
	linger_.l_onoff = 0;
	linger_.l_linger = 0;
	setsockopt(ctx->sfd, SOL_SOCKET, SO_LINGER, (const void*)&linger_, sizeof(linger_));

	/* connect to server */
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
	s_the_thread_data = _axtrace_try_init(server_ip, server_port);
	return (s_the_thread_data->is_init_succ != 0) ? s_the_thread_data : 0;
}

/*---------------------------------------------------------------------------------------------*/
void axlog(unsigned int style, const char *format, ...)
{
	axtrace_contex_s* ctx;
	va_list ptr = 0;
	HRESULT hr;
	size_t contents_byte_size, final_length;
	int send_len;

	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_log_s) + AXTRACE_MAX_TRACE_STRING_LENGTH] = { 0 };
	axtrace_log_s* trace_head = (axtrace_log_s*)(buf);
	char* trace_string = (char*)(buf + sizeof(axtrace_log_s));

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
	final_length = sizeof(axtrace_log_s)+contents_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_LOG;
	trace_head->head.pid = GetCurrentProcessId();
	trace_head->head.tid = GetCurrentThreadId();
	trace_head->head.style = style;

	trace_head->code_page = ATC_ACP;	/* TODO: get current system code page*/
	trace_head->length = (unsigned short)contents_byte_size;

	/* send to axtrace server*/
	send_len = send(ctx->sfd, buf, (int)final_length, MSG_DONTROUTE);

	/*TODO: check result, may be reconnect to server */
	return;
}

/*---------------------------------------------------------------------------------------------*/
static HRESULT _get_value_length(unsigned int value_type, const void* value, size_t* length)
{
	HRESULT hr;
	size_t l;

	switch (value_type)
	{
	case AXV_INT8: case AXV_UINT8: *length = 1; return S_OK;
	case AXV_INT16: case AXV_UINT16: *length = 2; return S_OK;
	case AXV_INT32: case AXV_UINT32: *length = 4; return S_OK;
	case AXV_INT64: case AXV_UINT64: *length = 8; return S_OK;
	case AXV_FLOAT32: *length = 4; return S_OK;
	case AXV_FLOAT64: *length = 8; return S_OK;
	case AXV_STR_ACP: case AXV_STR_UTF8:
	{
		hr = StringCbLengthA((const char*)value, AXTRACE_MAX_VALUE_LENGTH - 1, &l);
		if (FAILED(hr)) return hr;
		*length = l + 1;
		return S_OK;
	}
	case AXV_STR_UTF16:
	{
		hr = StringCbLengthW((const wchar_t*)value, AXTRACE_MAX_VALUE_LENGTH - 1, &l);
		if (FAILED(hr)) return hr;
		*length = l + 2;
		return S_OK;
	}
	default: break;
	}
	return E_FAIL;
}

/*---------------------------------------------------------------------------------------------*/
void axvalue(unsigned int style, unsigned int value_type, const char* value_name, const void* value)
{
	axtrace_contex_s* ctx;
	HRESULT hr;
	size_t value_name_length;
	size_t value_length;
	int send_len;
	size_t final_length;

	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_value_s) + AXTRACE_MAX_VALUENAME_LENGTH + AXTRACE_MAX_VALUE_LENGTH] = { 0 };
	axtrace_value_s* trace_head = (axtrace_value_s*)(buf);
	char* value_name_buf = (char*)(buf + sizeof(axtrace_value_s));

	/* is init ok? */
	ctx = _axtrace_get_thread_contex(0, 0);
	if (ctx == 0) return;

	/** get value name length*/
	if (value_name == 0) return;
	hr = StringCbLengthA(value_name, AXTRACE_MAX_VALUENAME_LENGTH - 1, &value_name_length);
	if (FAILED(hr)) return;
	/* add '\0' ended */
	value_name_length += 1;
	if (value_name_length <= 0 || value_name_length >= AXTRACE_MAX_VALUENAME_LENGTH) return;

	if (value == 0) return;
	hr = _get_value_length(value_type, value, &value_length);
	if (FAILED(hr)) return;
	if (value_length <= 0 || value_length >= AXTRACE_MAX_VALUE_LENGTH) return;

	/*calc final length */
	final_length = sizeof(axtrace_value_s) + value_name_length + value_length;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_VALUE;
	trace_head->head.pid = GetCurrentProcessId();
	trace_head->head.tid = GetCurrentThreadId();
	trace_head->head.style = style;

	trace_head->value_type = value_type;
	trace_head->name_len = (unsigned short)value_name_length;
	trace_head->value_len = (unsigned short)value_length;

	/* fill the value data */
	memcpy(value_name_buf, value_name, value_name_length);
	memcpy(value_name_buf + value_name_length, value, value_length);

	/* send to axtrace server*/
	send_len = send(ctx->sfd, buf, (int)final_length, MSG_DONTROUTE);

	/*TODO: check result, may be reconnect to server */
	return;

}

/*---------------------------------------------------------------------------------------------*/
void ax2d_clean_map(const char* map_name, double x_size, double y_size)
{
	axtrace_contex_s* ctx;
	HRESULT hr;
	size_t map_name_size, final_length;
	int send_len;

	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_2d_clean_map_s) + AXTRACE_MAX_MAP_NAME_LENGTH] = { 0 };
	axtrace_2d_clean_map_s* trace_head = (axtrace_2d_clean_map_s*)(buf);
	char* _name = (char*)(buf + sizeof(axtrace_2d_clean_map_s));

	/* is init ok? */
	ctx = _axtrace_get_thread_contex(0, 0);
	if (ctx == 0) return;

	/* copy map name */
	hr = StringCbCopyA(_name, AXTRACE_MAX_MAP_NAME_LENGTH, map_name);
	/* failed ?*/
	if (FAILED(hr)) return;

	/** get string length*/
	hr = StringCbLengthA(_name, AXTRACE_MAX_MAP_NAME_LENGTH - 1, &map_name_size);
	/* failed ?*/
	if (FAILED(hr)) return;

	/* add '\0' ended */
	map_name_size += 1;

	final_length = sizeof(axtrace_2d_clean_map_s) + map_name_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_CLEAN_MAP;
	trace_head->head.pid = GetCurrentProcessId();
	trace_head->head.tid = GetCurrentThreadId();
	trace_head->head.style = 0;

	trace_head->x_size = x_size;
	trace_head->y_size = y_size;
	trace_head->name_len = (unsigned short)map_name_size;

	/* send to axtrace server*/
	send_len = send(ctx->sfd, buf, (int)final_length, MSG_DONTROUTE);

	/*TODO: check result, may be reconnect to server */
	return;
}

/*---------------------------------------------------------------------------------------------*/
void ax2d_actor(const char* map_name, unsigned int actor_type, double x, double y, double dir)
{
	//TODO: ...
}
