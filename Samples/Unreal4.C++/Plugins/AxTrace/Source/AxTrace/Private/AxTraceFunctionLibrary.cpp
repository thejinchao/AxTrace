/***************************************************

					AXIA|Trace3

		(C) Copyright  Jean(www.thecodeway.com). 2016
***************************************************/

#include "AxTracePrivatePCH.h"
#include "AxTraceFunctionLibrary.h"

#include "Networking.h"
#include "SharedPointer.h"


//---------------------------------------------------------------------------------------------------------
#define DEFAULT_AXTRACE_SERVER_IP		"127.0.0.1"
#define DEFAULT_AXTRACE_SERVER_PORT		(1978)
#define AXTRACE_MAX_TRACE_STRING_LENGTH	(0x8000)
#define AXTRACE_MAX_VALUENAME_LENGTH	(128)
#define AXTRACE_MAX_VALUE_LENGTH		(1024)

#define AXTRACE_CMD_TYPE_TRACE		(1)
#define AXTRACE_CMD_TYPE_VALUE		(2)

#define ATC_ACP		(0)	//Default Windows ANSI code page.
#define ATC_UTF8	(1)	//Unicode 8
#define ATC_UTF16	(2)	//Unicode 16


//---------------------------------------------------------------------------------------------------------
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

/* axtrace trace data struct*/
typedef struct
{
	axtrace_head_s	head;			/* common head */
	unsigned short	code_page;		/* code page */
	unsigned short	length;			/* trace string length */

									/* [trace string data with '\0' ended] */
} axtrace_trace_s;

typedef struct
{
	axtrace_head_s	head;			/* common head */
	unsigned int	value_type;		/* value type AXV_* */
	unsigned short	name_len;		/* length of value name */
	unsigned short	value_len;		/* length of value */

									/* [name buf  with '\0' ended]*/
									/* [value buf] */
} axtrace_value_s;

#pragma pack(pop)

//---------------------------------------------------------------------------------------------------------
struct UAxTraceFunctionLibrary::AxTraceContext
{
	bool			is_init_succ;			
	FIPv4Address	address;		// axtrace server address
	FSocket*		socket;			// socket
};

//---------------------------------------------------------------------------------------------------------
UAxTraceFunctionLibrary::AxTraceContext* UAxTraceFunctionLibrary::_try_init(void)
{
	AxTraceContext* ctx = (AxTraceContext*)FMemory::Malloc(sizeof(AxTraceContext));
	if (ctx == 0)	{
		return 0;
	}

	FIPv4Address::Parse(TEXT(DEFAULT_AXTRACE_SERVER_IP), ctx->address);

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ctx->address.Value);
	addr->SetPort(DEFAULT_AXTRACE_SERVER_PORT);

	ctx->socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
	bool connected = ctx->socket->Connect(*addr);
	if (!connected) {
		ctx->socket->Close();
		ctx->is_init_succ = false;
		return ctx;
	}

	ctx->is_init_succ = true;
	return ctx;
}

//---------------------------------------------------------------------------------------------------------
UAxTraceFunctionLibrary::AxTraceContext* UAxTraceFunctionLibrary::_get_thread_context(void)
{
	static thread_local AxTraceContext* s_the_context = 0;

	if (s_the_context != 0) {
		/* already try init in this thread */
		return (s_the_context->is_init_succ) ? s_the_context : 0;
	}

	/* try init */
	s_the_context = _try_init();
	return (s_the_context->is_init_succ) ? s_the_context : 0;
}


//---------------------------------------------------------------------------------------------------------
void UAxTraceFunctionLibrary::AxLog(const FString& String, AxTraceStyleEnum Style, int UserDefineStyle)
{
	AxTraceContext* ctx = _get_thread_context();
	if (ctx == 0) return;

	// buf for send , call send() once
	char buf[sizeof(axtrace_trace_s) + AXTRACE_MAX_TRACE_STRING_LENGTH] = { 0 };
	axtrace_trace_s* trace_head = (axtrace_trace_s*)(buf);
	char* trace_string = (char*)(buf + sizeof(axtrace_trace_s));

	//get string length
	const char *paramMemory = (const char*)TCHAR_TO_UTF8(String.GetCharArray().GetData());
	int32 contents_byte_size = strlen(paramMemory);
	contents_byte_size += 1; // add '\0' ended
	FMemory::Memcpy(trace_string, paramMemory, contents_byte_size);

	// fill the trace head data
	size_t final_length = sizeof(axtrace_trace_s) + contents_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_TRACE;
	trace_head->head.pid = FPlatformProcess::GetCurrentProcessId();
	trace_head->head.tid = FPlatformTLS::GetCurrentThreadId();
	trace_head->head.style = ((Style== AxTraceStyleEnum::AXT_USERDEFINE) ? UserDefineStyle : (int)Style );

	trace_head->code_page = ATC_UTF8;
	trace_head->length = (unsigned short)contents_byte_size;

	int32 sent = 0;
	ctx->socket->Send((const uint8*)buf, final_length, sent);

	return;
}

//---------------------------------------------------------------------------------------------------------
void UAxTraceFunctionLibrary::AxValue(const FString& Name, const FString& Value, AxTraceStyleEnum Style, int UserDefineStyle)
{
	#define AXV_STR_UTF8	(11)

	AxTraceContext* ctx = _get_thread_context();
	if (ctx == 0) return;

	// buf for send , call send() once
	char buf[sizeof(axtrace_value_s) + AXTRACE_MAX_VALUENAME_LENGTH + AXTRACE_MAX_VALUE_LENGTH] = { 0 };
	axtrace_value_s* trace_head = (axtrace_value_s*)(buf);
	char* value_name_buf = (char*)(buf + sizeof(axtrace_value_s));

	//get name length
	const char *nameMemory = (const char*)TCHAR_TO_UTF8(Name.GetCharArray().GetData());
	int32 name_byte_size = strlen(nameMemory);
	name_byte_size += 1; // add '\0' ended
	if (name_byte_size <= 0 || name_byte_size >= AXTRACE_MAX_VALUENAME_LENGTH) return;

	//get value length
	const char *valueMemory = (const char*)TCHAR_TO_UTF8(Value.GetCharArray().GetData());
	int32 value_byte_size = strlen(valueMemory);
	value_byte_size += 1; // add '\0' ended
	if (value_byte_size <= 0 || value_byte_size >= AXTRACE_MAX_VALUE_LENGTH) return;

	/*calc final length */
	size_t final_length = sizeof(axtrace_value_s) + name_byte_size + value_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_VALUE;
	trace_head->head.pid = FPlatformProcess::GetCurrentProcessId();
	trace_head->head.tid = FPlatformTLS::GetCurrentThreadId();
	trace_head->head.style = ((Style == AxTraceStyleEnum::AXT_USERDEFINE) ? UserDefineStyle : (int)Style); 

	trace_head->value_type = AXV_STR_UTF8;
	trace_head->name_len = (unsigned short)name_byte_size;
	trace_head->value_len = (unsigned short)value_byte_size;

	FMemory::Memcpy(value_name_buf, nameMemory, name_byte_size);
	FMemory::Memcpy(value_name_buf+ name_byte_size, valueMemory, value_byte_size);

	int32 sent = 0;
	ctx->socket->Send((const uint8*)buf, final_length, sent);

	return;
}

