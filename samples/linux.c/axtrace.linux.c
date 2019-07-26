#include "axtrace.linux.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

/*---------------------------------------------------------------------------------------------*/
#define DEFAULT_AXTRACE_SERVER_IP		"127.0.0.1"
#define DEFAULT_AXTRACE_SERVER_PORT		(1978)
#define AXTRACE_MAX_TRACE_STRING_LENGTH	(0x8000)
#define AXTRACE_MAX_VALUENAME_LENGTH	(128)
#define AXTRACE_MAX_VALUE_LENGTH		(1024)
#define AXTRACE_MAX_SCENE_NAME_LENGTH	(128)
#define AXTRACE_MAX_SCENE_DEFINE_LENGTH	(2048)
#define AXTRACE_MAX_ACTOR_INFO_LENGTH	(2048)

#define AXTRACE_CMD_TYPE_LOG			(1)
#define AXTRACE_CMD_TYPE_VALUE			(2)
#define AXTRACE_CMD_TYPE_2D_BEGIN_SCENE	(3)
#define AXTRACE_CMD_TYPE_2D_ACTOR		(4)
#define AXTRACE_CMD_TYPE_2D_END_SCENE	(5)

/*---------------------------------------------------------------------------------------------*/
/* AxTrace Global data  */
typedef struct
{
	int		is_init_succ;			/* 0 means not, 1 means yes*/
	struct	sockaddr_in address;		/* axtrace server address */
	int		sfd;					/* socket file desc*/
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
} axtrace_head_s;

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
	long long		actor_id;		/* id of actor */
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

#pragma pack(pop)

/*---------------------------------------------------------------------------------------------*/
static axtrace_contex_s* _axtrace_try_init(const char* server_ip, unsigned short server_port)
{
    const char* final_server_ip = server_ip ? server_ip : DEFAULT_AXTRACE_SERVER_IP;
    unsigned short findal_server_port = server_port != 0 ? server_port : DEFAULT_AXTRACE_SERVER_PORT;
    
	axtrace_contex_s* ctx = (axtrace_contex_s*)malloc(sizeof(axtrace_contex_s));
	if (ctx == 0){
		//TODO: fatal error, should stop the process
		return 0;
	}
	memset(ctx, 0, sizeof(axtrace_contex_s));
	
	/* create address struct */
	ctx->address.sin_family = AF_INET;
	ctx->address.sin_port = htons(server_port != 0 ? server_port : DEFAULT_AXTRACE_SERVER_PORT);
	if (inet_pton(AF_INET, server_ip ? server_ip : DEFAULT_AXTRACE_SERVER_IP, &(ctx->address.sin_addr)) != 1) {
		return ctx;
	}
	
	/* create socket */
	ctx->sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (ctx->sfd < 0) {
		return ctx;
	}	
	
	/* set SO_LINGER off, make sure all data in send buf can be sended */
	struct linger linger_;
	linger_.l_onoff = 0;
	linger_.l_linger = 0;
	setsockopt(ctx->sfd, SOL_SOCKET, SO_LINGER, (const void*)&linger_, sizeof(linger_));
	
	/* connect to axtrace server */
	if (connect(ctx->sfd, (const struct sockaddr*)&(ctx->address), sizeof(struct sockaddr_in)) < 0) {
		close(ctx->sfd); 
		ctx->sfd = 0;
		return ctx;
	}
	/* init success */
	ctx->is_init_succ = 1;
	return ctx;
}

/*---------------------------------------------------------------------------------------------*/
static axtrace_contex_s* _axtrace_get_thread_contex(const char* server_ip, unsigned short server_port)
{
	static __thread axtrace_contex_s* s_the_thread_data = 0;

	if (s_the_thread_data != 0) {
		/* already try init in this thread */
		return (s_the_thread_data->is_init_succ != 0) ? s_the_thread_data : 0;
	}

	/* try init */
	s_the_thread_data = _axtrace_try_init(server_ip, server_port);
	return (s_the_thread_data->is_init_succ != 0) ? s_the_thread_data : 0;	
}

/*---------------------------------------------------------------------------------------------*/
void axlog(unsigned int log_type, const char *format, ...)
{
	axtrace_contex_s* ctx;
	va_list ptr;
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
	contents_byte_size = vsnprintf(trace_string, AXTRACE_MAX_TRACE_STRING_LENGTH, format, ptr);
	va_end(ptr);
	/* failed ?*/
	if (contents_byte_size < 0 ) return;	
	
	/* add '\0' ended */
	contents_byte_size += 1;

	/* fill the trace head data */
	final_length = sizeof(axtrace_log_s) + contents_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_LOG;
	trace_head->head.pid = getpid();
	trace_head->head.tid = syscall(SYS_gettid);

	trace_head->log_type = log_type;
	trace_head->code_page = ATC_UTF8;	/* TODO: get current system code page*/
	trace_head->length = (unsigned short)contents_byte_size;

	/* send to axtrace server*/
	send_len = write(ctx->sfd, buf, (int)final_length);
	
	/*TODO: check result, may be reconnect to server */
	return;
}

/*---------------------------------------------------------------------------------------------*/
static size_t _get_value_length(unsigned int value_type, const void* value)
{
	size_t l;

	switch (value_type)
	{
	case AXV_INT8: case AXV_UINT8:  return 1;
	case AXV_INT16: case AXV_UINT16: return 2;
	case AXV_INT32: case AXV_UINT32: return 4;
	case AXV_INT64: case AXV_UINT64: return 8;
	case AXV_FLOAT32: return 4;
	case AXV_FLOAT64: return 8;
	case AXV_STR_UTF8:
		{
			l = strnlen((const char*)value, AXTRACE_MAX_VALUE_LENGTH - 1);
			return l + 1;
		}
	case AXV_STR_UTF32:
		{
			l = wcsnlen((const wchar_t*)value, AXTRACE_MAX_VALUE_LENGTH/sizeof(wchar_t)-1);
			return (l + 1)*sizeof(wchar_t);
		}
	default: break;
	}
	return -1;
}

/*---------------------------------------------------------------------------------------------*/
void axvalue(unsigned int value_type, const char* value_name, const void* value)
{
	axtrace_contex_s* ctx;
	size_t value_name_length;
	size_t value_length;
	int send_len;
	size_t final_length;
	unsigned short* temp;

	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_value_s) + AXTRACE_MAX_VALUENAME_LENGTH + AXTRACE_MAX_VALUE_LENGTH] = { 0 };
	axtrace_value_s* trace_head = (axtrace_value_s*)(buf);
	char* value_name_buf = (char*)(buf + sizeof(axtrace_value_s));

	/* is init ok? */
	ctx = _axtrace_get_thread_contex(0, 0);
	if (ctx == 0) return;

	/** get value name length*/
	if (value_name == 0) return;
	value_name_length = strnlen(value_name, AXTRACE_MAX_VALUENAME_LENGTH - 1);
	/* add '\0' ended */
	value_name_length += 1;
	if (value_name_length <= 0 || value_name_length >= AXTRACE_MAX_VALUENAME_LENGTH) return;

	if (value == 0) return;
	value_length = _get_value_length(value_type, value);
	if (value_length <= 0 || value_length >= AXTRACE_MAX_VALUE_LENGTH) return;

	/*calc final length */
	final_length = sizeof(axtrace_value_s) + value_name_length + value_length;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_VALUE;
	trace_head->head.pid = getpid();
	trace_head->head.tid = syscall(SYS_gettid);

	trace_head->value_type = value_type;
	trace_head->name_len = (unsigned short)value_name_length;
	trace_head->value_len = (unsigned short)value_length;

	/* fill the value data */
	memcpy(value_name_buf, value_name, value_name_length);
	memcpy(value_name_buf + value_name_length, value, value_length);

	/* send to axtrace server*/
	send_len = write(ctx->sfd, buf, (int)final_length);

	/*TODO: check result, may be reconnect to server */
	return;
}

/*---------------------------------------------------------------------------------------------*/
void ax2d_begin_scene(const char* scene_name, double left, double top, double right, double bottom, const char* scene_define)
{
    axtrace_contex_s* ctx;
	size_t scene_name_size, scene_define_size, final_length;
	int send_len;
	
	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_2d_begin_scene_s) + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_SCENE_DEFINE_LENGTH] = { 0 };
	axtrace_2d_begin_scene_s* trace_head = (axtrace_2d_begin_scene_s*)(buf);
	char* _name = (char*)(buf + sizeof(axtrace_2d_begin_scene_s));

	/* is init ok? */
	ctx = _axtrace_get_thread_contex(0, 0);
	if (ctx == 0) return;

	/** get scene name length*/
	if (scene_name == 0) return;
	scene_name_size = strnlen(scene_name, AXTRACE_MAX_SCENE_NAME_LENGTH - 1);
	/* add '\0' ended */
	scene_name_size += 1;
	if (scene_name_size <= 0 || scene_name_size >= AXTRACE_MAX_SCENE_NAME_LENGTH) return;
    memcpy(_name, scene_name, scene_name_size);
    
	/* to scene define point */
	if (scene_define != 0) {
		_name = (char*)(buf + sizeof(axtrace_2d_begin_scene_s) + scene_name_size);
        
        /** get scene define length */
        scene_define_size = strnlen(scene_define, AXTRACE_MAX_SCENE_DEFINE_LENGTH - 1);

		/* add '\0' ended */
		scene_define_size += 1;
		if (scene_define_size >= AXTRACE_MAX_SCENE_DEFINE_LENGTH) return;
		    
		memcpy(_name, scene_define, scene_define_size);
	}
	else {
		scene_define_size = 0;
	}
	
	final_length = sizeof(axtrace_2d_begin_scene_s) + scene_name_size + scene_define_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_BEGIN_SCENE;
	trace_head->head.pid = getpid();
	trace_head->head.tid = syscall(SYS_gettid);

	trace_head->left = left;
	trace_head->top = top;
	trace_head->right = right;
	trace_head->bottom = bottom;
	trace_head->name_len = (unsigned short)scene_name_size;
	trace_head->define_len = (unsigned short)scene_define_size;

	/* send to axtrace server*/
	send_len = write(ctx->sfd, buf, (int)final_length);	
	
	return;
}

/*---------------------------------------------------------------------------------------------*/
void ax2d_actor(const char* scene_name, long long actor_id, double x, double y, double dir, unsigned int actor_style, const char* actor_info)
{
    axtrace_contex_s* ctx;
	size_t scene_name_size, actor_info_size, final_length;
	int send_len;

	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_2d_actor_s) + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_ACTOR_INFO_LENGTH] = { 0 };
	axtrace_2d_actor_s* trace_head = (axtrace_2d_actor_s*)(buf);
	char* _name = (char*)(buf + sizeof(axtrace_2d_actor_s));	    
    
	/* is init ok? */
	ctx = _axtrace_get_thread_contex(0, 0);
	if (ctx == 0) return;
	        
	/** get scene name length*/
	if (scene_name == 0) return;
	scene_name_size = strnlen(scene_name, AXTRACE_MAX_SCENE_NAME_LENGTH - 1);
	/* add '\0' ended */
	scene_name_size += 1;
	if (scene_name_size <= 0 || scene_name_size >= AXTRACE_MAX_SCENE_NAME_LENGTH) return;
    memcpy(_name, scene_name, scene_name_size);
    
	/* to scene define point */
	if (actor_info != 0) {
		_name = (char*)(buf + sizeof(axtrace_2d_actor_s) + scene_name_size);
        
        /** get actor info length */
        actor_info_size = strnlen(actor_info, AXTRACE_MAX_ACTOR_INFO_LENGTH - 1);

		/* add '\0' ended */
		actor_info_size += 1;
		if (actor_info_size >= AXTRACE_MAX_ACTOR_INFO_LENGTH) return;
		    
		memcpy(_name, actor_info, actor_info_size);
	}
	else {
		actor_info_size = 0;
	}
	
	final_length = sizeof(axtrace_2d_actor_s) + scene_name_size + actor_info_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_ACTOR;
	trace_head->head.pid = getpid();
	trace_head->head.tid = syscall(SYS_gettid);

	trace_head->actor_id = actor_id;
	trace_head->x = x;
	trace_head->y = y;
	trace_head->dir = dir;
	trace_head->style = actor_style;
	trace_head->name_len = (unsigned short)scene_name_size;
	trace_head->info_len = (unsigned short)actor_info_size;
	
	/* send to axtrace server*/
	send_len = write(ctx->sfd, buf, (int)final_length);	
	
	return;
}

/*---------------------------------------------------------------------------------------------*/
void ax2d_end_scene(const char* scene_name)
{
    axtrace_contex_s* ctx;
	size_t scene_name_size, scene_define_size, final_length;
	int send_len;
	
	/* buf for send , call send() once*/
	char buf[sizeof(axtrace_2d_end_scene_s) + AXTRACE_MAX_SCENE_NAME_LENGTH] = { 0 };
	axtrace_2d_end_scene_s* trace_head = (axtrace_2d_end_scene_s*)(buf);
	char* _name = (char*)(buf + sizeof(axtrace_2d_end_scene_s));	    
    
    /* is init ok? */
	ctx = _axtrace_get_thread_contex(0, 0);
	if (ctx == 0) return;
    
	/** get scene name length*/
	if (scene_name == 0) return;
	scene_name_size = strnlen(scene_name, AXTRACE_MAX_SCENE_NAME_LENGTH - 1);
	/* add '\0' ended */
	scene_name_size += 1;
	if (scene_name_size <= 0 || scene_name_size >= AXTRACE_MAX_SCENE_NAME_LENGTH) return;
    memcpy(_name, scene_name, scene_name_size);
    
	final_length = sizeof(axtrace_2d_end_scene_s) + scene_name_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_END_SCENE;
	trace_head->head.pid = getpid();
	trace_head->head.tid = syscall(SYS_gettid);

	trace_head->name_len = (unsigned short)scene_name_size;
	
	/* send to axtrace server*/
	send_len = write(ctx->sfd, buf, (int)final_length);	    
    
    return;
}
