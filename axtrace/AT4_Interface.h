﻿/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#pragma once

/*---------------------------------------------------------------------------------------------*/
#define AXTRACE_PROTO_VERSION	(4)

#define AXT_TRACE	(0)
#define AXT_DEBUG	(1)
#define AXT_INFO	(2)
#define AXT_WARN	(3)
#define AXT_ERROR	(4)
#define AXT_FATAL	(5)
#define AXT_USERDEF	(10)

#define ATC_ACP		(0)	//Default Windows ANSI code page.
#define ATC_UTF8	(1)	//Unicode 8
#define ATC_UTF16	(2)	//Unicode 16

#define AXV_INT8		(0)
#define AXV_UINT8		(1)
#define AXV_INT16		(2)
#define AXV_UINT16		(3)
#define AXV_INT32		(4)
#define AXV_UINT32		(5)
#define AXV_INT64		(6)
#define AXV_UINT64		(7)
#define AXV_FLOAT32		(8)
#define AXV_FLOAT64		(9)
#define AXV_STR_ACP		(10)
#define AXV_STR_UTF8	(11)
#define AXV_STR_UTF16	(12)
#define AXV_STR_UTF32	(13)
#define AXV_USER_DEF	(100)

#define AXTRACE_CMD_TYPE_SHAKEHAND		(0)
#define AXTRACE_CMD_TYPE_LOG			(1)
#define AXTRACE_CMD_TYPE_VALUE			(2)
#define AXTRACE_CMD_TYPE_2D_BEGIN_SCENE	(3)
#define AXTRACE_CMD_TYPE_2D_ACTOR		(4)
#define AXTRACE_CMD_TYPE_2D_END_SCENE	(5)
#define AXTRACE_CMD_TYPE_2D_ACTOR_LOG	(6)

#define AXTRACE_MAX_PROCESSNAME_LENGTH	(512)
#define AXTRACE_MAX_LOG_STRING_LENGTH	(0x8000)
#define AXTRACE_MAX_VALUENAME_LENGTH	(128)
#define AXTRACE_MAX_VALUE_LENGTH		(1024)
#define AXTRACE_MAX_SCENE_NAME_LENGTH	(128)
#define AXTRACE_MAX_SCENE_DEFINE_LENGTH	(2048)
#define AXTRACE_MAX_ACTOR_INFO_LENGTH	(2048)
#define AXTRACE_MAX_ACTOR_LOG_LENGTH	(2048)

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

struct MessageTime
{
	quint16	hour;
	quint16	minute;
	quint16	second;
	quint16	milliseconds;
};

#pragma pack(pop)
