/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/

#include "axtrace.win.h"

#if defined(_MSC_VER)
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <strsafe.h>
#include <Shlwapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")
#endif

/*---------------------------------------------------------------------------------------------*/
#define DEFAULT_AXTRACE_SERVER_IP		"127.0.0.1"
#define DEFAULT_AXTRACE_SERVER_PORT		(1978)
#define AXTRACE_MAX_PROCESSNAME_LENGTH	(512)
#define AXTRACE_MAX_LOG_STRING_LENGTH	(0x8000)
#define AXTRACE_MAX_VALUENAME_LENGTH	(128)
#define AXTRACE_MAX_VALUE_LENGTH		(1024)
#define AXTRACE_MAX_SCENE_NAME_LENGTH	(128)
#define AXTRACE_MAX_SCENE_DEFINE_LENGTH	(2048)
#define AXTRACE_MAX_ACTOR_INFO_LENGTH	(2048)
#define AXTRACE_MAX_ACTOR_LOG_LENGTH	(2048)

#define AXTRACE_PROTO_VERSION			(4)

#define AXTRACE_CMD_TYPE_SHAKEHAND		(0)
#define AXTRACE_CMD_TYPE_LOG			(1)
#define AXTRACE_CMD_TYPE_VALUE			(2)
#define AXTRACE_CMD_TYPE_2D_BEGIN_SCENE	(3)
#define AXTRACE_CMD_TYPE_2D_ACTOR		(4)
#define AXTRACE_CMD_TYPE_2D_END_SCENE	(5)
#define AXTRACE_CMD_TYPE_2D_ACTOR_LOG	(6)

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
} axtrace_head_s;

typedef struct  
{
	axtrace_head_s	head;			/* common head */
	unsigned short	ver;			/* proto ver */
	unsigned short	sname_len;		/* length of session name */
	unsigned int	pid;			/* process id*/
	unsigned int	tid;			/* thread id*/

									/* [session name buf  with '\0' ended]*/
} axtrace_shakehand_s;

/* axtrace log data struct*/
typedef struct
{
	axtrace_head_s	head;			/* common head */
	unsigned int	log_type;		/* trace style AXT_* */
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

typedef struct
{
	axtrace_head_s	head;			/* common head */
	double			left;			/* left of scene*/
	double			top;			/* top of scene*/
	double			right;			/* right of scene*/
	double			bottom;			/* bottom of scene*/
	unsigned short	name_len;		/* length of scene name */
	unsigned short	define_len;		/* length of scene define */

									/* [scene name buf  with '\0' ended]*/
									/* [scene define buf  with '\0' ended]*/
} axtrace_2d_begin_scene_s;

typedef struct
{
	axtrace_head_s	head;			/* common head */
	__int64			actor_id;		/* id of actor */
	double			x;				/* position (x)*/
	double			y;				/* position (y)*/
	double			dir;			/* direction */
	unsigned int	style;			/* user define style */
	unsigned short	name_len;		/* length of actor name */
	unsigned short	info_len;		/* length of actor information */

									/* [actor name buf  with '\0' ended]*/
									/* [actor information buf  with '\0' ended]*/
} axtrace_2d_actor_s;

typedef struct
{
	axtrace_head_s	head;			/* common head */
	unsigned short	name_len;		/* length of scene name */

									/* [scene name buf  with '\0' ended]*/
} axtrace_2d_end_scene_s;

typedef struct
{
	axtrace_head_s	head;			/* common head */
	__int64			actor_id;		/* id of actor */
	unsigned short	name_len;		/* length of scene name */
	unsigned short	log_len;		/* length of actor log */

									/* [scene name buf  with '\0' ended]*/
									/* [actor log(utf8) buf  with '\0' ended]*/
} axtrace_2d_actor_log_s;

#pragma pack(pop)

/*---------------------------------------------------------------------------------------------*/
static void _send_handshake_message(axtrace_contex_s* ctx)
{
	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_shakehand_s) + AXTRACE_MAX_PROCESSNAME_LENGTH] = { 0 };
	wchar_t wszProcessModuleName[MAX_PATH] = { 0 };
	axtrace_shakehand_s* shakehand_head = (axtrace_shakehand_s*)(buf);
	char* pname_string = (char*)(buf + sizeof(axtrace_shakehand_s));
	int pname_length = 0;
	int send_len;
	size_t final_length = 0;

	DWORD dwPNameLengthAsChar = GetModuleFileNameW(NULL, wszProcessModuleName, MAX_PATH-1);
	if (dwPNameLengthAsChar > 0)
	{
		pname_length = WideCharToMultiByte(CP_UTF8, 0, PathFindFileNameW(wszProcessModuleName), dwPNameLengthAsChar, pname_string, AXTRACE_MAX_PROCESSNAME_LENGTH - 1, NULL, NULL);
	}
	if (pname_length == 0)
	{
		pname_string[0] = 0; /* fill '\0' as empty process name */
	}
	
	/* add '\0' ended */
	pname_length += 1;

	/*calc final length*/
	final_length = sizeof(axtrace_shakehand_s) + pname_length;

	shakehand_head->head.length = (unsigned short)(final_length);
	shakehand_head->head.flag = 'A';
	shakehand_head->head.type = AXTRACE_CMD_TYPE_SHAKEHAND;

	shakehand_head->ver = AXTRACE_PROTO_VERSION;
	shakehand_head->sname_len = pname_length;
	shakehand_head->pid = GetCurrentProcessId();
	shakehand_head->tid = GetCurrentThreadId();

	/* send to axtrace server*/
	send_len = send(ctx->sfd, buf, (int)final_length, MSG_DONTROUTE);

	return;
}

/*---------------------------------------------------------------------------------------------*/
static axtrace_contex_s* _axtrace_try_init(const char* server_ip, unsigned short server_port)
{
	axtrace_contex_s* ctx = (axtrace_contex_s*)LocalAlloc(LPTR, sizeof(axtrace_contex_s));
	if (ctx == 0)
	{
		//TODO: fatal error, should stop the process
		return 0;
	}
	memset(ctx, 0, sizeof(axtrace_contex_s));

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 1), &wsadata);

	ctx->address.sin_family = AF_INET;
	ctx->address.sin_port = htons(server_port);
	if (0 == InetPtonA(AF_INET, server_ip, &(ctx->address.sin_addr)))
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
	setsockopt(ctx->sfd, SOL_SOCKET, SO_LINGER, (const char*)&linger_, sizeof(linger_));

	/* connect to server */
	if (connect(ctx->sfd, (const struct sockaddr*)&(ctx->address), sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		closesocket(ctx->sfd);
		return ctx;
	}
	/* send hand shake message */
	_send_handshake_message(ctx);

	/* init success */
	ctx->is_init_succ = 1;
	return ctx;
}

/*---------------------------------------------------------------------------------------------*/
static axtrace_contex_s* _axtrace_get_thread_contex()
{
	static __declspec(thread) axtrace_contex_s* s_the_thread_data = 0;

	if (s_the_thread_data != 0) {
		/* already try init in this thread */
		return (s_the_thread_data->is_init_succ != 0) ? s_the_thread_data : 0;
	}

	/* try init */
	s_the_thread_data = _axtrace_try_init(DEFAULT_AXTRACE_SERVER_IP, DEFAULT_AXTRACE_SERVER_PORT);
	return (s_the_thread_data->is_init_succ != 0) ? s_the_thread_data : 0;
}

/*---------------------------------------------------------------------------------------------*/
void axlog(unsigned int log_type, const char *format, ...)
{
	axtrace_contex_s* ctx;
	va_list ptr = 0;
	HRESULT hr;
	size_t contents_byte_size, final_length;
	int send_len;

	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_log_s) + AXTRACE_MAX_LOG_STRING_LENGTH] = { 0 };
	axtrace_log_s* trace_head = (axtrace_log_s*)(buf);
	char* trace_string = (char*)(buf + sizeof(axtrace_log_s));

	/* is init ok? */
	ctx = _axtrace_get_thread_contex();
	if (ctx == 0) return;

	/* Create String Contents*/
	va_start(ptr, format);
	hr = StringCbVPrintfA(trace_string, AXTRACE_MAX_LOG_STRING_LENGTH, format, ptr);
	va_end(ptr);
	/* failed ?*/
	if (FAILED(hr)) return;

	/** get string length*/
	hr = StringCbLengthA(trace_string, AXTRACE_MAX_LOG_STRING_LENGTH - 1, &contents_byte_size);
	/* failed ?*/
	if (FAILED(hr)) return;

	/* add '\0' ended */
	contents_byte_size += 1;

	/* fill the trace head data */
	final_length = sizeof(axtrace_log_s) + contents_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_LOG;

	trace_head->log_type = log_type;
	trace_head->code_page = ATC_ACP;
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
void axvalue(unsigned int value_type, const char* value_name, const void* value)
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
	ctx = _axtrace_get_thread_contex();
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
void ax2d_begin_scene(const char* scene_name, double left, double top, double right, double bottom, const char* scene_define)
{
	axtrace_contex_s* ctx;
	HRESULT hr;
	size_t scene_name_size, scene_define_size, final_length;
	int send_len;

	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_2d_begin_scene_s) + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_SCENE_DEFINE_LENGTH] = { 0 };
	axtrace_2d_begin_scene_s* trace_head = (axtrace_2d_begin_scene_s*)(buf);
	char* _name = (char*)(buf + sizeof(axtrace_2d_begin_scene_s));

	/* is init ok? */
	ctx = _axtrace_get_thread_contex();
	if (ctx == 0) return;

	/* copy scene name */
	hr = StringCbCopyA(_name, AXTRACE_MAX_SCENE_NAME_LENGTH, scene_name);
	/* failed ?*/
	if (FAILED(hr)) return;

	/** get string length*/
	hr = StringCbLengthA(_name, AXTRACE_MAX_SCENE_NAME_LENGTH - 1, &scene_name_size);
	/* failed ?*/
	if (FAILED(hr)) return;
	if (scene_name_size <= 0 || scene_name_size >= AXTRACE_MAX_SCENE_NAME_LENGTH) return;

	/* add '\0' ended */
	scene_name_size += 1;

	/* to scene define point */
	if (scene_define != 0) {
		_name = (char*)(buf + sizeof(axtrace_2d_begin_scene_s) + scene_name_size);

		/* copy scene define */
		hr = StringCbCopyA(_name, AXTRACE_MAX_SCENE_DEFINE_LENGTH, scene_define);
		/* failed ?*/
		if (FAILED(hr)) return;

		/** get string length*/
		hr = StringCbLengthA(_name, AXTRACE_MAX_SCENE_DEFINE_LENGTH - 1, &scene_define_size);
		/* failed ?*/
		if (FAILED(hr)) return;
		if (scene_define_size >= AXTRACE_MAX_SCENE_DEFINE_LENGTH) return;

		/* add '\0' ended */
		scene_define_size += 1;
	}
	else {
		scene_define_size = 0;
	}

	final_length = sizeof(axtrace_2d_begin_scene_s) + scene_name_size + scene_define_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_BEGIN_SCENE;

	trace_head->left = left;
	trace_head->top = top;
	trace_head->right = right;
	trace_head->bottom = bottom;
	trace_head->name_len = (unsigned short)scene_name_size;
	trace_head->define_len = (unsigned short)scene_define_size;

	/* send to axtrace server*/
	send_len = send(ctx->sfd, buf, (int)final_length, MSG_DONTROUTE);

	/*TODO: check result, may be reconnect to server */
	return;
}

/*---------------------------------------------------------------------------------------------*/
void ax2d_actor(const char* scene_name, __int64 actor_id, double x, double y, double dir, unsigned int actor_style, const char* actor_info)
{
	axtrace_contex_s* ctx;
	HRESULT hr;
	size_t actor_name_size, actor_info_size, final_length;
	int send_len;

	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_2d_actor_s) + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_ACTOR_INFO_LENGTH] = { 0 };
	axtrace_2d_actor_s* trace_head = (axtrace_2d_actor_s*)(buf);
	char* _name = (char*)(buf + sizeof(axtrace_2d_actor_s));

	/* is init ok? */
	ctx = _axtrace_get_thread_contex();
	if (ctx == 0) return;

	/* copy scene name */
	hr = StringCbCopyA(_name, AXTRACE_MAX_SCENE_NAME_LENGTH, scene_name);
	/* failed ?*/
	if (FAILED(hr)) return;

	/** get actor name string length*/
	hr = StringCbLengthA(_name, AXTRACE_MAX_SCENE_NAME_LENGTH - 1, &actor_name_size);
	/* failed ?*/
	if (FAILED(hr)) return;
	if (actor_name_size <= 0 || actor_name_size >= AXTRACE_MAX_SCENE_NAME_LENGTH) return;

	/* add '\0' ended */
	actor_name_size += 1;

	/* to scene define point */
	if (actor_info != 0) {
		_name = (char*)(buf + sizeof(axtrace_2d_actor_s) + actor_name_size);

		/* copy scene define */
		hr = StringCbCopyA(_name, AXTRACE_MAX_ACTOR_INFO_LENGTH, actor_info);
		/* failed ?*/
		if (FAILED(hr)) return;

		/** get string length*/
		hr = StringCbLengthA(_name, AXTRACE_MAX_ACTOR_INFO_LENGTH - 1, &actor_info_size);
		/* failed ?*/
		if (FAILED(hr)) return;
		if (actor_info_size >= AXTRACE_MAX_ACTOR_INFO_LENGTH) return;

		/* add '\0' ended */
		actor_info_size += 1;
	}
	else {
		actor_info_size = 0;
	}

	final_length = sizeof(axtrace_2d_actor_s) + actor_name_size + actor_info_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_ACTOR;

	trace_head->actor_id = actor_id;
	trace_head->x = x;
	trace_head->y = y;
	trace_head->dir = dir;
	trace_head->style = actor_style;
	trace_head->name_len = (unsigned short)actor_name_size;
	trace_head->info_len = (unsigned short)actor_info_size;

	/* send to axtrace server*/
	send_len = send(ctx->sfd, buf, (int)final_length, MSG_DONTROUTE);

	/*TODO: check result, may be reconnect to server */
	return;
}

/*---------------------------------------------------------------------------------------------*/
void ax2d_end_scene(const char* scene_name)
{
	axtrace_contex_s* ctx;
	HRESULT hr;
	size_t scene_name_size, final_length;
	int send_len;

	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_2d_end_scene_s) + AXTRACE_MAX_SCENE_NAME_LENGTH] = { 0 };
	axtrace_2d_end_scene_s* trace_head = (axtrace_2d_end_scene_s*)(buf);
	char* _name = (char*)(buf + sizeof(axtrace_2d_end_scene_s));

	/* is init ok? */
	ctx = _axtrace_get_thread_contex();
	if (ctx == 0) return;

	/* copy scene name */
	hr = StringCbCopyA(_name, AXTRACE_MAX_SCENE_NAME_LENGTH, scene_name);
	/* failed ?*/
	if (FAILED(hr)) return;

	/** get string length*/
	hr = StringCbLengthA(_name, AXTRACE_MAX_SCENE_NAME_LENGTH - 1, &scene_name_size);
	/* failed ?*/
	if (FAILED(hr)) return;

	/* add '\0' ended */
	scene_name_size += 1;

	final_length = sizeof(axtrace_2d_end_scene_s) + scene_name_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_END_SCENE;

	trace_head->name_len = (unsigned short)scene_name_size;

	/* send to axtrace server*/
	send_len = send(ctx->sfd, buf, (int)final_length, MSG_DONTROUTE);

	/*TODO: check result, may be reconnect to server */
	return;
}

/*---------------------------------------------------------------------------------------------*/
void ax2d_actor_log(const char* scene_name, __int64 actor_id, const char* actor_log)
{
	axtrace_contex_s* ctx;
	HRESULT hr;
	size_t scene_name_size, actor_log_size, final_length;
	int send_len;

	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_2d_actor_log_s) + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_ACTOR_LOG_LENGTH] = { 0 };
	axtrace_2d_actor_log_s* trace_head = (axtrace_2d_actor_log_s*)(buf);
	char* _buf = (char*)(buf + sizeof(axtrace_2d_actor_log_s));

	/* is init ok? */
	ctx = _axtrace_get_thread_contex();
	if (ctx == 0) return;

	/* check param */
	if (scene_name == 0 || scene_name[0] == '\0') return;
	if (actor_log == 0 || actor_log[0] == '\0') return;

	/* copy scene name */
	hr = StringCbCopyA(_buf, AXTRACE_MAX_SCENE_NAME_LENGTH, scene_name);
	/* failed ?*/
	if (FAILED(hr)) return;

	/** get string length*/
	hr = StringCbLengthA(_buf, AXTRACE_MAX_SCENE_NAME_LENGTH - 1, &scene_name_size);
	/* failed ?*/
	if (FAILED(hr)) return;

	/* add '\0' ended */
	scene_name_size += 1;

	_buf = (char*)(buf + sizeof(axtrace_2d_actor_log_s) + scene_name_size);

	/* copy actor log */
	hr = StringCbCopyA(_buf, AXTRACE_MAX_ACTOR_LOG_LENGTH, actor_log);
	/* failed ?*/
	if (FAILED(hr)) return;

	/** get string length*/
	hr = StringCbLengthA(_buf, AXTRACE_MAX_ACTOR_LOG_LENGTH - 1, &actor_log_size);
	/* failed ?*/
	if (FAILED(hr)) return;

	/* add '\0' ended */
	actor_log_size += 1;

	final_length = sizeof(axtrace_2d_actor_log_s) + scene_name_size + actor_log_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_ACTOR_LOG;

	trace_head->actor_id = actor_id;
	trace_head->name_len = (unsigned short)scene_name_size;
	trace_head->log_len = (unsigned short)actor_log_size;

	/* send to axtrace server*/
	send_len = send(ctx->sfd, buf, (int)final_length, MSG_DONTROUTE);

	return;
}