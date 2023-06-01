/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#ifndef __AXIA_TRACE_WINDOWS_INCLUDE__
#define __AXIA_TRACE_WINDOWS_INCLUDE__

#ifdef __cplusplus
#define AXTRACE_EXTERN_C extern "C"
#else
#define AXTRACE_EXTERN_C extern
#endif

#define AXT_TRACE	(0)
#define AXT_DEBUG	(1)
#define AXT_INFO	(2)
#define AXT_WARN	(3)
#define AXT_ERROR	(4)
#define AXT_FATAL	(5)
#define AXT_USERDEF	(10)

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
#define AXV_USER_DEF	(100)

/*
* send a log message to axtrace server
* @param log_type is one of AXT_*** value,
* @param format is the message described with system current codec
*
*	sample: axlog(AXT_TRACE, "hello,world! I'm %s", name);
*/
AXTRACE_EXTERN_C void axlog(unsigned int log_type, const char *format, ...);

/*
* watch a value
* @param value_type is one of AXV_*** value,
* @param value_name is the name of value, ended with '\0'
* @param value is the memory address of value, the size of value is decided by value_type
*/
AXTRACE_EXTERN_C void axvalue(unsigned int value_type, const char* value_name, const void* value);


/**
* begin draw a 2d scene

x_min                         x_max
+----------------------------+  y_min
|                            |
|                            |
|                            |
|                            |
|                            |
+----------------------------+  y_max

@param scene_name the name of scene(id)
@param x_min left of scene
@param y_min top of scene
@param x_max right of scene
@param y_max bottom of scene
@param scene_define extra define of scene, It's json object
*/
AXTRACE_EXTERN_C void ax2d_begin_scene(const char* scene_name, double x_min, double y_min, double x_max, double y_max, const char* scene_define);

/*
create/update a actor in the scene

+-------------+--------------+
|             ^              |
|             | y            |
|     x       v              |
+<----------> O              |
|                            |
|                            |
+----------------------------+

@param scene_name the name of scene(id)
@param actor_id actor id
@param x actor position(x)
@param y actor position(y)
@param dir actor direction(0~2pi)
@param actor_style user define style
@param actor_info extra information of actor
*/
AXTRACE_EXTERN_C void ax2d_actor(const char* scene_name, __int64 actor_id, double x, double y, double dir, unsigned int actor_style, const char* actor_info);

/*
draw all actors between ax2d_begin_scene and ax2d_end_scene
@param scene_name the name of scene(id)
*/
AXTRACE_EXTERN_C void ax2d_end_scene(const char* scene_name);

/*
push log to a actor, It can be called any time, not necessarily between begin_scene and end_scene
@param scene_name the name of scene(id)
@param actor_id actor id
@param actor_log actor log
*/
AXTRACE_EXTERN_C void ax2d_actor_log(const char* scene_name, __int64 actor_id, const char* actor_log);
#endif
