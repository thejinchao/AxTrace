/***************************************************

					AXIA|Trace4

		(C) Copyright  www.thecodeway.com 2023
***************************************************/

#include "AxTraceFunctionLibrary.h"
#include "AxTraceSettings.h"
#include "AxTraceModule.h"
#include "Networking.h"
#include "Templates/SharedPointer.h"

//---------------------------------------------------------------------------------------------------------
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
#define AXV_USER_DEF	(100)

//---------------------------------------------------------------------------------------------------------
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

/* axtrace trace data struct*/
typedef struct
{
	axtrace_head_s	head;			/* common head */
	unsigned int	log_type;		/* trace style AXT_* */
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

typedef struct
{
	axtrace_head_s	head;			/* common head */
	double			x_min;			/* left of scene*/
	double			y_min;			/* top of scene*/
	double			x_max;			/* right of scene*/
	double			y_max;			/* bottom of scene*/
	unsigned short	name_len;		/* length of scene name */
	unsigned short	define_len;		/* length of scene define */

									/* [scene name buf  with '\0' ended]*/
									/* [scene define buf  with '\0' ended]*/
} axtrace_2d_begin_scene_s;

typedef struct
{
	axtrace_head_s	head;			/* common head */
	int64			actor_id;		/* id of actor */
	double			x;				/* position (x)*/
	double			y;				/* position (y)*/
	double			dir;			/* direction */
	unsigned int	style;			/* user define style */
	unsigned short	name_len;		/* length of scene name */
	unsigned short	info_len;		/* length of actor information */

									/* [scene name buf  with '\0' ended]*/
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
	int64			actor_id;		/* id of actor */
	unsigned short	name_len;		/* length of scene name */
	unsigned short	log_len;		/* length of actor log */

									/* [scene name buf  with '\0' ended]*/
									/* [actor log(utf8) buf  with '\0' ended]*/
} axtrace_2d_actor_log_s;

#pragma pack(pop)

struct UAxTrace::AxTraceContext
{
	bool			is_init_succ;	// is init successed?
	int				pieID;			// id of PIE
	FIPv4Address	address;		// axtrace server address
	FSocket*		socket;			// socket
};

//---------------------------------------------------------------------------------------------------------
void UAxTrace::ResetContext(UAxTrace::AxTraceContext* ctx)
{
	if (ctx != nullptr && ctx->socket != nullptr) {
		ctx->socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ctx->socket);
		ctx->socket = nullptr;
	}
}

//---------------------------------------------------------------------------------------------------------
UAxTrace::AxTraceContext* UAxTrace::CreateContext()
{
	AxTraceContext* ctx = (AxTraceContext*)FMemory::Malloc(sizeof(AxTraceContext));
	if (ctx == nullptr) return nullptr;

	const UAxTraceSettings* settings = FAxTraceModule::Get()->GeAxTraceConfig();
	if (!(settings->EnableAxTrace)) {
		ctx->is_init_succ = false;
		return ctx;
	}

	FIPv4Address::Parse(settings->ReceiverAddress, ctx->address);

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ctx->address.Value);
	addr->SetPort(settings->ReceiverPort);

	ctx->socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
	bool connected = ctx->socket->Connect(*addr);
	if (!connected) {
		ResetContext(ctx);

		ctx->is_init_succ = false;
		return ctx;
	}

	// send hand shake message
	SendShakehandMessage(ctx);
	ctx->is_init_succ = true;
	return ctx;
}

//---------------------------------------------------------------------------------------------------------
UAxTrace::AxTraceContext* UAxTrace::GetContext()
{
#if UE_BUILD_SHIPPING || UE_BUILD_TEST
	return nullptr;
#else
	static thread_local UAxTrace::AxTraceContext* s_the_context = nullptr;

	if (s_the_context != nullptr) {

		if (s_the_context->pieID != FAxTraceModule::Get()->GetPIEID()) {
			/* close current connection first */
			delete s_the_context; s_the_context = nullptr;
		}
		else {
			/* already try init in this thread */
			return (s_the_context->is_init_succ) ? s_the_context : nullptr;
		}
	}

	/* try init */
	s_the_context = CreateContext();
	s_the_context->pieID = FAxTraceModule::Get()->GetPIEID();
	return (s_the_context->is_init_succ) ? s_the_context : 0;
#endif
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::SendShakehandMessage(UAxTrace::AxTraceContext* ctx)
{
	// buf for send
	char buf[sizeof(axtrace_shakehand_s) + AXTRACE_MAX_PROCESSNAME_LENGTH] = { 0 };
	axtrace_shakehand_s* shakehand_head = (axtrace_shakehand_s*)(buf);
	char* sname_string = (char*)(buf + sizeof(axtrace_shakehand_s));

	// get game name
	const char* projectName = TCHAR_TO_UTF8(FApp::GetProjectName());
	size_t prjNameLen = strlen(projectName);

	FMemory::Memcpy(sname_string, projectName, prjNameLen);
	/* add '\0' ended */
	prjNameLen += 1;

	/*calc final length*/
	size_t final_length = sizeof(axtrace_shakehand_s) + prjNameLen;

	shakehand_head->head.length = (unsigned short)(final_length);
	shakehand_head->head.flag = 'A';
	shakehand_head->head.type = AXTRACE_CMD_TYPE_SHAKEHAND;

	shakehand_head->ver = AXTRACE_PROTO_VERSION;
	shakehand_head->sname_len = prjNameLen;
	shakehand_head->pid = FPlatformProcess::GetCurrentProcessId();
	shakehand_head->tid = FPlatformTLS::GetCurrentThreadId();

	int32 sent = 0;
	ctx->socket->Send((const uint8*)buf, final_length, sent);

	return;
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Log(const FString& String, AxTraceStyleEnum Style, int UserDefineStyle)
{
	AxTraceContext* ctx = GetContext();
	if (ctx == nullptr) return;

	// buf for send , call send() once
	char buf[sizeof(axtrace_trace_s) + AXTRACE_MAX_LOG_STRING_LENGTH] = { 0 };
	axtrace_trace_s* trace_head = (axtrace_trace_s*)(buf);
	char* trace_string = (char*)(buf + sizeof(axtrace_trace_s));

	//get string length
	const TCHAR* strMemory = *String;
	int32 contents_byte_size = (String.Len()+1)*sizeof(TCHAR); // add '\0' ended
	if (contents_byte_size >= AXTRACE_MAX_LOG_STRING_LENGTH)
	{
		contents_byte_size = AXTRACE_MAX_LOG_STRING_LENGTH;
		FMemory::Memcpy(trace_string, strMemory, contents_byte_size-sizeof(TCHAR));
		FMemory::Memset(trace_string + AXTRACE_MAX_LOG_STRING_LENGTH - sizeof(TCHAR), 0, sizeof(TCHAR));
	}
	else
	{
		FMemory::Memcpy(trace_string, strMemory, contents_byte_size);
	}

	// fill the trace head data
	size_t final_length = sizeof(axtrace_trace_s) + contents_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_LOG;

	trace_head->code_page = ATC_UTF16;
	trace_head->log_type = ((Style == AxTraceStyleEnum::AXT_USERDEFINE) ? UserDefineStyle : (int)Style);
	trace_head->length = (unsigned short)contents_byte_size;

	int32 sent = 0;
	ctx->socket->Send((const uint8*)buf, final_length, sent);

	return;
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_Boolean(FString Name, bool Value)
{
	const static char TrueString[] = "True";
	const static char FalseString[] = "False";

	_Value(Name, (Value ? TrueString : FalseString), AXV_STR_UTF8, (Value ? 5 : 6));
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_Int8(FString Name, int8 Value)
{
	char valueMemory[sizeof(Value)];
	memcpy(valueMemory, &Value, sizeof(Value));

	_Value(Name, valueMemory, AXV_INT8, sizeof(Value));
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_UInt8(FString Name, uint8 Value)
{
	char valueMemory[sizeof(Value)];
	memcpy(valueMemory, &Value, sizeof(Value));

	_Value(Name, valueMemory, AXV_UINT8, sizeof(Value));
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_Int32(FString Name, int32 Value)
{
	char valueMemory[sizeof(Value)];
	memcpy(valueMemory, &Value, sizeof(Value));

	_Value(Name, valueMemory, AXV_INT32, sizeof(Value));
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_UInt32(FString Name, uint32 Value)
{
	char valueMemory[sizeof(Value)];
	memcpy(valueMemory, &Value, sizeof(Value));

	_Value(Name, valueMemory, AXV_UINT32, sizeof(Value));
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_Int64(FString Name, int64 Value)
{
	char valueMemory[sizeof(Value)];
	memcpy(valueMemory, &Value, sizeof(Value));

	_Value(Name, valueMemory, AXV_INT64, sizeof(Value));
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_UInt64(FString Name, uint64 Value)
{
	char valueMemory[sizeof(Value)];
	memcpy(valueMemory, &Value, sizeof(Value));

	_Value(Name, valueMemory, AXV_UINT64, sizeof(Value));
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_Float(FString Name, float Value)
{
	char valueMemory[sizeof(Value)];
	memcpy(valueMemory, &Value, sizeof(Value));

	_Value(Name, valueMemory, AXV_FLOAT32, sizeof(Value));
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_Double(FString Name, double Value)
{
	char valueMemory[sizeof(Value)];
	memcpy(valueMemory, &Value, sizeof(Value));

	_Value(Name, valueMemory, AXV_FLOAT64, sizeof(Value));
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_String(FString Name, FString Value)
{
	const TCHAR *valueMemory = *Value;
	int32 value_byte_size = (Value.Len()+1)*sizeof(TCHAR);
	if (value_byte_size <= 0 || value_byte_size >= AXTRACE_MAX_VALUE_LENGTH) return;

	_Value(Name, valueMemory, AXV_STR_UTF16, value_byte_size);
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::Value_Name(FString Name, FName Value)
{
	Value_String(Name, Value.ToString());
}

//---------------------------------------------------------------------------------------------------------
void UAxTrace::_Value(const FString& Name, const void* Value, unsigned int ValueType, int ValueSize)
{
	AxTraceContext* ctx = GetContext();
	if (ctx == nullptr) return;

	// buf for send , call send() once
	char buf[sizeof(axtrace_value_s) + AXTRACE_MAX_VALUENAME_LENGTH + AXTRACE_MAX_VALUE_LENGTH] = { 0 };
	axtrace_value_s* trace_head = (axtrace_value_s*)(buf);
	char* value_name_buf = (char*)(buf + sizeof(axtrace_value_s));

	//get name length
	const char* nameMemory = (const char*)TCHAR_TO_UTF8(Name.GetCharArray().GetData());
	int32 name_byte_size = strlen(nameMemory);
	name_byte_size += 1; // add '\0' ended
	if (name_byte_size <= 0 || name_byte_size >= AXTRACE_MAX_VALUENAME_LENGTH) return;

	//check value length
	int value_byte_size = ValueSize;
	if (value_byte_size <= 0 || value_byte_size >= AXTRACE_MAX_VALUE_LENGTH) return;

	/*calc final length */
	size_t final_length = sizeof(axtrace_value_s) + name_byte_size + value_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_VALUE;

	trace_head->value_type = ValueType;
	trace_head->name_len = (unsigned short)name_byte_size;
	trace_head->value_len = (unsigned short)value_byte_size;

	FMemory::Memcpy(value_name_buf, nameMemory, name_byte_size);
	FMemory::Memcpy(value_name_buf + name_byte_size, Value, value_byte_size);

	int32 sent = 0;
	ctx->socket->Send((const uint8*)buf, final_length, sent);

	return;
}

void UAxTrace::Scene2DBegin(const FString& SceneName, float xMin, float yMin, float xMax, float yMax, const FString& SceneDefine)
{
	AxTraceContext* ctx = GetContext();
	if (ctx == nullptr) return;

	// buf for send , call send() once
	char buf[sizeof(axtrace_2d_begin_scene_s) + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_SCENE_DEFINE_LENGTH] = { 0 };
	axtrace_2d_begin_scene_s* trace_head = (axtrace_2d_begin_scene_s*)(buf);
	char* scene_name_buf = (char*)(buf + sizeof(axtrace_2d_begin_scene_s));

	//get name length
	const char *nameMemory = (const char*)TCHAR_TO_UTF8(SceneName.GetCharArray().GetData());
	if (nameMemory == nullptr) return;
	int32 name_byte_size = strlen(nameMemory);
	name_byte_size += 1; // add '\0' ended
	if (name_byte_size <= 0 || name_byte_size >= AXTRACE_MAX_SCENE_NAME_LENGTH) return;

	//get define length
	int32 define_byte_size = 0;
	
	#define SCENEDEFINE_TCHAR_TO_UTF8(str) (ANSICHAR*)TStringConversion<FTCHARToUTF8_Convert, AXTRACE_MAX_SCENE_DEFINE_LENGTH>((const TCHAR*)str).Get()
	const char *defineMemory = (const char*)SCENEDEFINE_TCHAR_TO_UTF8(*SceneDefine);
	if (defineMemory) {
		define_byte_size = strlen(defineMemory);
		define_byte_size += 1; // add '\0' ended
		if (define_byte_size <= 0 || define_byte_size >= AXTRACE_MAX_SCENE_DEFINE_LENGTH) return;
	}

	/*calc final length */
	size_t final_length = sizeof(axtrace_2d_begin_scene_s) + name_byte_size + define_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_BEGIN_SCENE;

	trace_head->x_min = xMin;
	trace_head->y_min = yMin;
	trace_head->x_max = xMax;
	trace_head->y_max = yMax;
	trace_head->name_len = (unsigned short)name_byte_size;
	trace_head->define_len = (unsigned short)define_byte_size;

	FMemory::Memcpy(scene_name_buf, nameMemory, name_byte_size);
	if (defineMemory) {
		FMemory::Memcpy(scene_name_buf + name_byte_size, defineMemory, define_byte_size);
	}

	int32 sent = 0;
	ctx->socket->Send((const uint8*)buf, final_length, sent);
}

void UAxTrace::Scene2DActor(const FString& SceneName, int64 ActorID, const FVector2D& Position, float Direction, int32 ActorStyle, const FString& ActorInfo)
{
	AxTraceContext* ctx = GetContext();
	if (ctx == nullptr) return;

	// buf for send , call send() once
	char buf[sizeof(axtrace_2d_actor_s) + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_ACTOR_INFO_LENGTH] = { 0 };
	axtrace_2d_actor_s* trace_head = (axtrace_2d_actor_s*)(buf);
	char* scene_name_buf = (char*)(buf + sizeof(axtrace_2d_actor_s));

	//get name length
	const char *nameMemory = (const char*)TCHAR_TO_UTF8(SceneName.GetCharArray().GetData());
	if (nameMemory == nullptr) return;
	int32 name_byte_size = strlen(nameMemory);
	name_byte_size += 1; // add '\0' ended
	if (name_byte_size <= 0 || name_byte_size >= AXTRACE_MAX_SCENE_NAME_LENGTH) return;

	//get actor info length
	int32 info_byte_size = 0;
	#define ACTORINFO_TCHAR_TO_UTF8(str) (ANSICHAR*)TStringConversion<FTCHARToUTF8_Convert, AXTRACE_MAX_ACTOR_INFO_LENGTH>((const TCHAR*)str).Get()
	const char *infoMemory = (const char*)ACTORINFO_TCHAR_TO_UTF8(ActorInfo.GetCharArray().GetData());
	if (infoMemory) {
		info_byte_size = strlen(infoMemory);
		info_byte_size += 1; // add '\0' ended
		if (info_byte_size <= 0 || info_byte_size >= AXTRACE_MAX_ACTOR_INFO_LENGTH) return;
	}

	/*calc final length */
	size_t final_length = sizeof(axtrace_2d_actor_s) + name_byte_size + info_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_ACTOR;

	trace_head->actor_id = ActorID;
	trace_head->x = Position.X;
	trace_head->y = Position.Y;
	trace_head->dir = Direction;
	trace_head->style = ActorStyle;
	trace_head->name_len = (unsigned short)name_byte_size;
	trace_head->info_len = (unsigned short)info_byte_size;

	FMemory::Memcpy(scene_name_buf, nameMemory, name_byte_size);
	if (infoMemory) {
		FMemory::Memcpy(scene_name_buf + name_byte_size, infoMemory, info_byte_size);
	}

	int32 sent = 0;
	ctx->socket->Send((const uint8*)buf, final_length, sent);
}

void UAxTrace::Scene2DEnd(const FString& SceneName)
{
	AxTraceContext* ctx = GetContext();
	if (ctx == nullptr) return;

	// buf for send , call send() once
	char buf[sizeof(axtrace_2d_end_scene_s) + AXTRACE_MAX_SCENE_NAME_LENGTH] = { 0 };
	axtrace_2d_end_scene_s* trace_head = (axtrace_2d_end_scene_s*)(buf);
	char* scene_name_buf = (char*)(buf + sizeof(axtrace_2d_end_scene_s));

	//get name length
	const char *nameMemory = (const char*)TCHAR_TO_UTF8(SceneName.GetCharArray().GetData());
	if (nameMemory == nullptr) return;
	int32 name_byte_size = strlen(nameMemory);
	name_byte_size += 1; // add '\0' ended
	if (name_byte_size <= 0 || name_byte_size >= AXTRACE_MAX_SCENE_NAME_LENGTH) return;

	/*calc final length */
	size_t final_length = sizeof(axtrace_2d_end_scene_s) + name_byte_size;

	trace_head->head.length = (unsigned short)(final_length);
	trace_head->head.flag = 'A';
	trace_head->head.type = AXTRACE_CMD_TYPE_2D_END_SCENE;

	trace_head->name_len = (unsigned short)name_byte_size;

	FMemory::Memcpy(scene_name_buf, nameMemory, name_byte_size);

	int32 sent = 0;
	ctx->socket->Send((const uint8*)buf, final_length, sent);
}
