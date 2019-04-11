#include "stdafx.h"
#include "AT4_Message.h"

//--------------------------------------------------------------------------------------------
Message::Message()
	: m_processID(0)
	, m_threadID(0)
{

}

//--------------------------------------------------------------------------------------------
Message::~Message()
{

}

//-------------------------------------------------------------------------------------
int Message::_lua_get_type(lua_State *L)
{
	const Message* msg = (const Message*)lua_touserdata(L, 1);
	lua_pushinteger(L, msg->getType());
	return 1;
}

//-------------------------------------------------------------------------------------
int Message::_lua_get_process_id(lua_State *L)
{
	const Message* msg = (const Message*)lua_touserdata(L, 1);
	lua_pushinteger(L, msg->getProcessID());
	return 1;
}

//-------------------------------------------------------------------------------------
int Message::_lua_get_thread_id(lua_State *L)
{
	const Message* msg = (const Message*)lua_touserdata(L, 1);
	lua_pushinteger(L, msg->getThreadID());
	return 1;
}

//--------------------------------------------------------------------------------------------
QQueue<LogMessage*> LogMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
LogMessage::LogMessage()
{

}

//--------------------------------------------------------------------------------------------
LogMessage::~LogMessage()
{

}

//--------------------------------------------------------------------------------------------
void LogMessage::build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	static QThreadStorage<QByteArray> memoryCache;

	memcpy(&m_time, &traceTime, sizeof(axtrace_time_s));
	m_processID = head.pid;
	m_threadID = head.tid;

	axtrace_log_s logHead;
	size_t len = ringBuf->memcpy_out(&logHead, sizeof(axtrace_log_s));
	assert(len == sizeof(axtrace_log_s));

	m_logType = logHead.log_type;

	int logLength = logHead.length;

	QByteArray& cache = memoryCache.localData();
	if (cache.size() < logLength + 2)
		cache.resize(logLength + 2);

	len = ringBuf->memcpy_out(cache.data(), logLength);
	assert(len == logLength);

	cache.data()[logLength] = 0;
	cache.data()[logLength+1] = 0;

	switch (logHead.code_page)
	{
	case ATC_UTF16:
		m_log = QString::fromUtf16((const ushort*)cache.data());
		break;
	case ATC_UTF8:
		m_log = QString::fromUtf8(cache);
		break;
	case ATC_ACP:
		m_log = QString::fromLocal8Bit(cache);
		break;
	}
}


//-------------------------------------------------------------------------------------
int LogMessage::_lua_get_log_type(lua_State *L)
{
	const LogMessage* msg = (const LogMessage*)lua_touserdata(L, 1);
	lua_pushinteger(L, msg->getLogType());
	return 1;
}

//-------------------------------------------------------------------------------------
int LogMessage::_lua_get_log(lua_State *L)
{
	const LogMessage* msg = (const LogMessage*)lua_touserdata(L, 1);

	QByteArray msgUtf8 = msg->getLog().toUtf8();
	lua_pushstring(L, msgUtf8.data());

	return 1;
}

//--------------------------------------------------------------------------------------------
const char* LogMessage::MetaName = "AxTrace.LogMessage";

void LogMessage::_luaopen(lua_State *L)
{
	static luaL_Reg msg_data_meta[] =
	{
		{ "get_type", Message::_lua_get_type },
		{ "get_pid", Message::_lua_get_process_id },
		{ "get_tid", Message::_lua_get_thread_id },

		{ "get_log_type", LogMessage::_lua_get_log_type },
		{ "get_log", LogMessage::_lua_get_log },

		{ 0, 0 }
	};


	//PlayerData meta table
	luaL_newmetatable(L, LogMessage::MetaName);
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */

	luaL_register(L, NULL, msg_data_meta);  /* file methods */
}

//--------------------------------------------------------------------------------------------
QQueue<ValueMessage*> ValueMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
ValueMessage::ValueMessage()
	: m_valueBuf(nullptr)
{

}

//--------------------------------------------------------------------------------------------
ValueMessage::~ValueMessage()
{
	if (m_valueBuf && m_valueBuf != m_standValueBuf) 
	{
		delete[] m_valueBuf;
	}
}

//--------------------------------------------------------------------------------------------
void ValueMessage::build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	memcpy(&m_time, &traceTime, sizeof(axtrace_time_s));
	m_processID = head.pid;
	m_threadID = head.tid;

	axtrace_value_s value_head;
	size_t len = ringBuf->memcpy_out(&value_head, sizeof(value_head));
	assert(len == sizeof(value_head));

	m_valueType = value_head.value_type;
	m_valueSize = value_head.value_len;

	//copy name 
	char tempName[AXTRACE_MAX_VALUENAME_LENGTH];
	int name_length = value_head.name_len;
	//TODO: check name length
	len = ringBuf->memcpy_out(tempName, name_length);
	assert(len == name_length);
	tempName[name_length - 1] = 0; //make sure last char is '\0'
	m_name = QString::fromUtf8(tempName);

	//value
	if (m_valueSize > STANDARD_VALUE_SIZE)
	{
		//big value
		m_valueBuf = new char[m_valueSize];
		memset(m_valueBuf, 0, m_valueSize);
	}
	else
	{
		m_valueBuf = m_standValueBuf;
	}

	//value
	len = ringBuf->memcpy_out(m_valueBuf, m_valueSize);
	assert(len == m_valueSize);

	//make sure '\0' ended
	if (m_valueType == AXV_STR_ACP || m_valueType == AXV_STR_UTF8)
	{
		((char*)m_valueBuf)[m_valueSize - 1] = 0;
	}
	else if (m_valueType == AXV_STR_UTF16)
	{
		((char*)m_valueBuf)[m_valueSize - 1] = 0;
		((char*)m_valueBuf)[m_valueSize - 2] = 0;
	}
}

//--------------------------------------------------------------------------------------------
void ValueMessage::getValueAsString(QString& value) const
{
	const static QString ERROR_VALUE("<ERR>");

	switch (m_valueType)
	{
	case AXV_INT8:
		value = QString::number(*((int8_t*)m_valueBuf));
		break;

	case AXV_UINT8:
		value = QString::number(*((uint8_t*)m_valueBuf));
		break;

	case AXV_INT16:
		value = QString::number(*((int16_t*)m_valueBuf));
		break;

	case AXV_UINT16:
		value = QString::number(*((uint16_t*)m_valueBuf));
		break;

	case AXV_INT32:
		value = QString::number(*((int32_t*)m_valueBuf));
		break;

	case AXV_UINT32:
		value = QString::number(*((uint32_t*)m_valueBuf));
		break;

	case AXV_INT64:
		value = QString::number(*((int64_t*)m_valueBuf));
		break;

	case AXV_UINT64:
		value = QString::number(*((uint64_t*)m_valueBuf));
		break;

	case AXV_FLOAT32:
		value = QString::number(*((float*)m_valueBuf));
		break;

	case AXV_FLOAT64:
		value = QString::number(*((double*)m_valueBuf));
		break;

	case AXV_STR_UTF16:
		value = QString::fromUtf16((const ushort*)m_valueBuf);
		break;

	case AXV_STR_UTF8:
		value = QString::fromUtf8((const char*)m_valueBuf);
		break;

	case AXV_STR_ACP:
		value = QString::fromLocal8Bit((const char*)m_valueBuf);
		break;

	default:
		value = ERROR_VALUE;
		break;
	}
}

//-------------------------------------------------------------------------------------
int ValueMessage::_lua_get_value(lua_State *L)
{
	const ValueMessage* msg = (const ValueMessage*)lua_touserdata(L, 1);

	QString value_as_string;
	msg->getValueAsString(value_as_string);

	QByteArray msgUtf8 = value_as_string.toUtf8();
	lua_pushstring(L, msgUtf8.data());
	return 1;
}

//--------------------------------------------------------------------------------------------
const char* ValueMessage::MetaName = "AxTrace.ValueMessage";

void ValueMessage::_luaopen(lua_State *L)
{
	static luaL_Reg msg_data_meta[] =
	{
		{ "get_type", Message::_lua_get_type },
		{ "get_pid", Message::_lua_get_process_id },
		{ "get_tid", Message::_lua_get_thread_id },

		{ "get_value", ValueMessage::_lua_get_value },

		{ 0, 0 }
	};


	//PlayerData meta table
	luaL_newmetatable(L, ValueMessage::MetaName);
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */

	luaL_register(L, NULL, msg_data_meta);  /* file methods */
}

//--------------------------------------------------------------------------------------------
QQueue<Begin2DSceneMessage*> Begin2DSceneMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
Begin2DSceneMessage::Begin2DSceneMessage()
{

}

//--------------------------------------------------------------------------------------------
Begin2DSceneMessage::~Begin2DSceneMessage()
{

}

//--------------------------------------------------------------------------------------------
void Begin2DSceneMessage::build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	memcpy(&m_time, &traceTime, sizeof(axtrace_time_s));
	m_processID = head.pid;
	m_threadID = head.tid;

	axtrace_2d_begin_scene_s value_head;
	size_t len = ringBuf->memcpy_out(&value_head, sizeof(value_head));
	assert(len == sizeof(value_head));

	m_sceneRect = QRectF(value_head.left, value_head.top, value_head.right-value_head.left, value_head.bottom-value_head.top);

	//copy name 
	char tempBuf[AXTRACE_MAX_SCENE_DEFINE_LENGTH];
	int name_length = value_head.name_len;
	//TODO: check name length
	len = ringBuf->memcpy_out(tempBuf, name_length);
	assert(len == name_length);
	tempBuf[name_length - 1] = 0; //make sure last char is '\0'
	m_sceneName = QString::fromUtf8(tempBuf);

	//copy define 
	int define_length = value_head.define_len;
	len = ringBuf->memcpy_out(tempBuf, define_length);
	assert(len == define_length);
	tempBuf[define_length] = 0;
	
	//make json object
	if (define_length > 0) 
	{
		QJsonParseError jerror;
		QJsonDocument jsonDocument = QJsonDocument::fromJson(tempBuf, &jerror);
		if (jerror.error == QJsonParseError::NoError)
		{
			m_sceneDefine = jsonDocument.object();
		}
	}
}

//--------------------------------------------------------------------------------------------
const char* Begin2DSceneMessage::MetaName = "AxTrace.Begin2DScene";

void Begin2DSceneMessage::_luaopen(lua_State *L)
{
	static luaL_Reg msg_data_meta[] =
	{
		{ "get_type", Message::_lua_get_type },
		{ "get_pid", Message::_lua_get_process_id },
		{ "get_tid", Message::_lua_get_thread_id },

		{ 0, 0 }
	};


	//PlayerData meta table
	luaL_newmetatable(L, Begin2DSceneMessage::MetaName);
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */

	luaL_register(L, NULL, msg_data_meta);  /* file methods */
}

//--------------------------------------------------------------------------------------------
QQueue<Update2DActorMessage*> Update2DActorMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
Update2DActorMessage::Update2DActorMessage()
{

}

//--------------------------------------------------------------------------------------------
Update2DActorMessage::~Update2DActorMessage()
{

}

//--------------------------------------------------------------------------------------------
void Update2DActorMessage::build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	memcpy(&m_time, &traceTime, sizeof(axtrace_time_s));
	m_processID = head.pid;
	m_threadID = head.tid;

	axtrace_2d_actor_s value_head;
	size_t len = ringBuf->memcpy_out(&value_head, sizeof(value_head));
	assert(len == sizeof(value_head));

	m_actorID = (qint64)value_head.actor_id;
	m_position = QPointF((qreal)value_head.x, (qreal)value_head.y);
	m_dir = (qreal)value_head.dir;
	m_style = (quint32)value_head.style;

	//copy name 
	char tempName[AXTRACE_MAX_SCENE_NAME_LENGTH];
	int name_length = value_head.name_len;
	//TODO: check name length
	len = ringBuf->memcpy_out(tempName, name_length);
	assert(len == name_length);
	tempName[name_length - 1] = 0; //make sure last char is '\0'
	m_sceneName = QString::fromUtf8(tempName);
}

//-------------------------------------------------------------------------------------
int Update2DActorMessage::_lua_get_actor_id(lua_State *L)
{
	const Update2DActorMessage* msg = (const Update2DActorMessage*)lua_touserdata(L, 1);

	lua_pushinteger(L, msg->getActorID());
	return 1;
}

//-------------------------------------------------------------------------------------
int Update2DActorMessage::_lua_get_actor_position(lua_State *L)
{
	const Update2DActorMessage* msg = (const Update2DActorMessage*)lua_touserdata(L, 1);

	const QPointF& pos = msg->getActorPosition();
	lua_pushnumber(L, pos.x());
	lua_pushnumber(L, pos.y());
	return 2;
}

//-------------------------------------------------------------------------------------
int Update2DActorMessage::_lua_get_actor_dir(lua_State *L)
{
	const Update2DActorMessage* msg = (const Update2DActorMessage*)lua_touserdata(L, 1);

	lua_pushnumber(L, msg->getActorDir());
	return 1;
}

//-------------------------------------------------------------------------------------
int Update2DActorMessage::_lua_get_actor_style(lua_State *L)
{
	const Update2DActorMessage* msg = (const Update2DActorMessage*)lua_touserdata(L, 1);

	lua_pushinteger(L, msg->getActorStyle());
	return 1;
}

//--------------------------------------------------------------------------------------------
const char* Update2DActorMessage::MetaName = "AxTrace.Actor2DMessage";

void Update2DActorMessage::_luaopen(lua_State *L)
{
	static luaL_Reg msg_data_meta[] =
	{
		{ "get_type", Message::_lua_get_type },
		{ "get_pid", Message::_lua_get_process_id },
		{ "get_tid", Message::_lua_get_thread_id },

		{ "get_actor_id", Update2DActorMessage::_lua_get_actor_id },
		{ "get_actor_position", Update2DActorMessage::_lua_get_actor_position },
		{ "get_actor_dir", Update2DActorMessage::_lua_get_actor_dir },
		{ "get_actor_style", Update2DActorMessage::_lua_get_actor_style },

		{ 0, 0 }
	};


	//PlayerData meta table
	luaL_newmetatable(L, Update2DActorMessage::MetaName);
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */

	luaL_register(L, NULL, msg_data_meta);  /* file methods */
}

//--------------------------------------------------------------------------------------------
QQueue<End2DSceneMessage*> End2DSceneMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
End2DSceneMessage::End2DSceneMessage()
{

}

//--------------------------------------------------------------------------------------------
End2DSceneMessage::~End2DSceneMessage()
{

}

//--------------------------------------------------------------------------------------------
void End2DSceneMessage::build(const axtrace_time_s& traceTime, const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	memcpy(&m_time, &traceTime, sizeof(axtrace_time_s));
	m_processID = head.pid;
	m_threadID = head.tid;

	axtrace_2d_end_scene_s value_head;
	size_t len = ringBuf->memcpy_out(&value_head, sizeof(value_head));
	assert(len == sizeof(value_head));

	//copy name 
	char tempName[AXTRACE_MAX_SCENE_NAME_LENGTH];
	int name_length = value_head.name_len;
	//TODO: check name length
	len = ringBuf->memcpy_out(tempName, name_length);
	assert(len == name_length);
	tempName[name_length - 1] = 0; //make sure last char is '\0'
	m_sceneName = QString::fromUtf8(tempName);
}

//--------------------------------------------------------------------------------------------
const char* End2DSceneMessage::MetaName = "AxTrace.End2DScene";

void End2DSceneMessage::_luaopen(lua_State *L)
{
	static luaL_Reg msg_data_meta[] =
	{
		{ "get_type", Message::_lua_get_type },
		{ "get_pid", Message::_lua_get_process_id },
		{ "get_tid", Message::_lua_get_thread_id },

		{ 0, 0 }
	};

	//PlayerData meta table
	luaL_newmetatable(L, End2DSceneMessage::MetaName);
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */

	luaL_register(L, NULL, msg_data_meta);  /* file methods */
}
