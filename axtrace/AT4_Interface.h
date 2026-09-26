/***************************************************

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
	uint16_t		length;			/* length */
	uint8_t			flag;			/* magic flag, always 'A' */
	uint8_t			type;			/* command type AXTRACE_CMD_TYPE_* */
} axtrace_head_s;

static_assert(sizeof(axtrace_head_s)==4);

typedef struct
{
	axtrace_head_s	head;			/* common head */
	uint16_t		ver;			/* proto ver */
	uint16_t		sname_len;		/* length of session name */
	uint32_t		pid;			/* process id*/
	uint32_t		tid;			/* thread id*/

									/* [session name buf  with '\0' ended]*/
} axtrace_shakehand_s;

static_assert(sizeof(axtrace_shakehand_s) == 16);

/* axtrace log data struct*/
typedef struct
{
	axtrace_head_s	head;			/* common head */
	uint32_t		log_type;		/* trace style AXT_* */
	uint16_t		code_page;		/* code page */
	uint16_t		length;			/* trace string length */

									/* [trace string data with '\0' ended] */
} axtrace_log_s;

static_assert(sizeof(axtrace_log_s) == 12);

typedef struct
{
	axtrace_head_s	head;			/* common head */
	uint32_t		value_type;		/* value type AXV_* */
	uint16_t		name_len;		/* length of value name */
	uint16_t		value_len;		/* length of value */

									/* [name buf  with '\0' ended]*/
									/* [value buf] */
} axtrace_value_s;

static_assert(sizeof(axtrace_value_s) == 12);

typedef struct
{
	axtrace_head_s	head;			/* common head */
	double			x_left;			/* left of scene*/
	double			y_top;			/* top of scene*/
	double			x_right;		/* right of scene*/
	double			y_bottom;		/* bottom of scene*/
	uint16_t		name_len;		/* length of scene name */
	uint16_t		define_len;		/* length of scene define */

									/* [scene name buf  with '\0' ended]*/
									/* [scene define buf  with '\0' ended]*/
} axtrace_2d_begin_scene_s;

static_assert(sizeof(axtrace_2d_begin_scene_s) == 40);

typedef struct
{
	axtrace_head_s	head;			/* common head */
	int64_t			actor_id;		/* id of actor */
	double			x;				/* position (x)*/
	double			y;				/* position (y)*/
	double			dir;			/* direction */
	uint32_t		style;			/* user define style */
	uint16_t		name_len;		/* length of actor name */
	uint16_t		info_len;		/* length of actor information */

									/* [actor name buf  with '\0' ended]*/
									/* [actor information buf  with '\0' ended]*/
} axtrace_2d_actor_s;

static_assert(sizeof(axtrace_2d_actor_s) == 44);

typedef struct
{
	axtrace_head_s	head;			/* common head */
	uint16_t		name_len;		/* length of scene name */

									/* [scene name buf  with '\0' ended]*/
} axtrace_2d_end_scene_s;

static_assert(sizeof(axtrace_2d_end_scene_s) == 6);

typedef struct
{
	axtrace_head_s	head;			/* common head */
	int64_t			actor_id;		/* id of actor */
	uint16_t		name_len;		/* length of scene name */
	uint16_t		log_len;		/* length of actor log */

									/* [scene name buf  with '\0' ended]*/
									/* [actor log(utf8) buf  with '\0' ended]*/
} axtrace_2d_actor_log_s;

static_assert(sizeof(axtrace_2d_actor_log_s) == 16);

#pragma pack(pop)
