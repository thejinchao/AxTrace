/***************************************************

					AXIA|Trace3

							(C) Copyright  Jean. 2013
***************************************************/
#pragma once

namespace AT3
{

/*---------------------------------------------------------------------------------------------*/
#define AXT_TRACE	(0)
#define AXT_DEBUG	(1)
#define AXT_INFO	(2)
#define AXT_WARN	(3)
#define AXT_ERROR	(4)
#define AXT_FATAL	(5)

#define AXT_USERDEF	(10)

#define AXTRACE_CMD_TYPE_TRACE		(1)
#define AXTRACE_CMD_TYPE_VALUE		(2)

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

}
