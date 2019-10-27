/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2019
***************************************************/
#include "stdafx.h"
#include "AT4_Message.h"

//--------------------------------------------------------------------------------------------
Message::Message(SessionPtr session, const axtrace_time_s& traceTime)
	: m_session(session)
{
	memcpy(&m_time, &traceTime, sizeof(axtrace_time_s));
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
	lua_pushinteger(L, msg->getSession()->getProcessID());
	return 1;
}

//-------------------------------------------------------------------------------------
int Message::_lua_get_thread_id(lua_State *L)
{
	const Message* msg = (const Message*)lua_touserdata(L, 1);
	lua_pushinteger(L, msg->getSession()->getThreadID());
	return 1;
}

//--------------------------------------------------------------------------------------------
QQueue<ShakehandMessage*> ShakehandMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
ShakehandMessage::ShakehandMessage(SessionPtr session, const axtrace_time_s& traceTime)
	: Message(session, traceTime)
{

}

//--------------------------------------------------------------------------------------------
ShakehandMessage::~ShakehandMessage()
{

}

//--------------------------------------------------------------------------------------------
bool ShakehandMessage::build(const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	axtrace_shakehand_s shakehand;
	size_t len = ringBuf->peek(0, &shakehand, sizeof(axtrace_shakehand_s));
	assert(len == sizeof(axtrace_shakehand_s));

	//check version
	if (shakehand.ver != AXTRACE_PROTO_VERSION) return false;
	//check name length
	if (shakehand.sname_len > AXTRACE_MAX_PROCESSNAME_LENGTH) return false;

	//receive session name
	char sessionName[AXTRACE_MAX_PROCESSNAME_LENGTH] = { 0 };
	len = ringBuf->peek(sizeof(axtrace_shakehand_s), sessionName, shakehand.sname_len);
	assert(len == shakehand.sname_len);

	sessionName[shakehand.sname_len - 1] = 0; //make sure last char is '\0'
	m_sessionName = QString::fromUtf8(sessionName);

	m_version = shakehand.ver;
	m_processID = shakehand.pid;
	m_threadID = shakehand.tid;

	//shakehand
	if (!(m_session->onSessionShakehand(this))) return false;

	//ok!
	ringBuf->discard(shakehand.head.length);

	return true;
}

//--------------------------------------------------------------------------------------------
QQueue<LogMessage*> LogMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
LogMessage::LogMessage(SessionPtr session, const axtrace_time_s& traceTime)
	: Message(session, traceTime)
{

}

//--------------------------------------------------------------------------------------------
LogMessage::~LogMessage()
{

}

//--------------------------------------------------------------------------------------------
bool LogMessage::build(const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	static QThreadStorage<QByteArray> memoryCache;

	if (!(m_session->isHandshaked())) return false;

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
	return true;
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
ValueMessage::ValueMessage(SessionPtr session, const axtrace_time_s& traceTime)
	: Message(session, traceTime)
	, m_valueBuf(nullptr)
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
bool ValueMessage::build(const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	if (!(m_session->isHandshaked())) return false;

	axtrace_value_s value_head;
	size_t len = ringBuf->peek(0, &value_head, sizeof(axtrace_value_s));
	assert(len == sizeof(value_head));

	m_valueType = value_head.value_type;
	m_valueSize = value_head.value_len;

	//copy name 
	char tempName[AXTRACE_MAX_VALUENAME_LENGTH];
	int name_length = value_head.name_len;
	//check name length
	if (name_length > AXTRACE_MAX_VALUENAME_LENGTH) return false;

	//ok
	ringBuf->discard(sizeof(axtrace_value_s));

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
	else if (m_valueType == AXV_STR_UTF32)
	{
		((char*)m_valueBuf)[m_valueSize - 1] = 0;
		((char*)m_valueBuf)[m_valueSize - 2] = 0;
		((char*)m_valueBuf)[m_valueSize - 3] = 0;
		((char*)m_valueBuf)[m_valueSize - 4] = 0;
	}
	return true;
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

	case AXV_STR_UTF32:
		value = QString::fromUcs4((const uint*)m_valueBuf);
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
Begin2DSceneMessage::Begin2DSceneMessage(SessionPtr session, const axtrace_time_s& traceTime)
	: Message(session, traceTime)
{

}

//--------------------------------------------------------------------------------------------
Begin2DSceneMessage::~Begin2DSceneMessage()
{

}

//--------------------------------------------------------------------------------------------
bool Begin2DSceneMessage::build(const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	if (!(m_session->isHandshaked())) return false;

	axtrace_2d_begin_scene_s value_head;
	size_t len = ringBuf->memcpy_out(&value_head, sizeof(value_head));
	assert(len == sizeof(value_head));

	m_sceneRect = QRectF(value_head.left, value_head.top, value_head.right-value_head.left, value_head.bottom-value_head.top);

	//copy name 
	char tempBuf[AXTRACE_MAX_SCENE_DEFINE_LENGTH] = { 0 };
	int name_length = value_head.name_len;
	if (name_length > AXTRACE_MAX_SCENE_NAME_LENGTH) name_length = AXTRACE_MAX_SCENE_NAME_LENGTH;
	if (name_length > 0) {
		len = ringBuf->memcpy_out(tempBuf, name_length);
		assert(len == name_length);
		tempBuf[name_length - 1] = 0; //make sure last char is '\0'
		m_sceneName = QString::fromUtf8(tempBuf);
	}

	//copy define 
	int define_length = value_head.define_len;
	if (define_length > AXTRACE_MAX_SCENE_DEFINE_LENGTH) define_length = AXTRACE_MAX_SCENE_DEFINE_LENGTH;
	if (define_length > 0) {
		len = ringBuf->memcpy_out(tempBuf, define_length);
		assert(len == define_length);
		tempBuf[define_length - 1] = 0;
	
		//make json object
		QJsonParseError jerror;
		QJsonDocument jsonDocument = QJsonDocument::fromJson(tempBuf, &jerror);
		if (jerror.error == QJsonParseError::NoError)
		{
			m_sceneDefine = jsonDocument.object();
		}
	}
	return true;
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
Update2DActorMessage::Update2DActorMessage(SessionPtr session, const axtrace_time_s& traceTime)
	: Message(session, traceTime)
{

}

//--------------------------------------------------------------------------------------------
Update2DActorMessage::~Update2DActorMessage()
{

}

//--------------------------------------------------------------------------------------------
bool Update2DActorMessage::build(const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	if (!(m_session->isHandshaked())) return false;

	axtrace_2d_actor_s value_head;
	size_t len = ringBuf->memcpy_out(&value_head, sizeof(value_head));
	assert(len == sizeof(value_head));

	m_actorID = (qint64)value_head.actor_id;
	m_position = QPointF((qreal)value_head.x, (qreal)value_head.y);
	m_dir = (qreal)value_head.dir;
	m_style = (quint32)value_head.style;

	//copy name 
	char tempName[AXTRACE_MAX_SCENE_NAME_LENGTH] = { 0 };
	int name_length = value_head.name_len;
	if (name_length > AXTRACE_MAX_SCENE_NAME_LENGTH) name_length = AXTRACE_MAX_SCENE_NAME_LENGTH;
	if (name_length > 0) {
		len = ringBuf->memcpy_out(tempName, name_length);
		assert(len == name_length);
		tempName[name_length - 1] = 0; //make sure last char is '\0'
		m_sceneName = QString::fromUtf8(tempName);
	}

	//copy info
	char tempInfo[AXTRACE_MAX_ACTOR_INFO_LENGTH] = { 0 };
	int info_length = value_head.info_len;
	if (info_length > AXTRACE_MAX_ACTOR_INFO_LENGTH) info_length = AXTRACE_MAX_ACTOR_INFO_LENGTH;
	if (info_length > 0) {
		len = ringBuf->memcpy_out(tempInfo, info_length);
		assert(len == info_length);
		tempInfo[info_length - 1] = 0; //make sure last char is '\0'
		m_actorInfo = QString::fromUtf8(tempInfo);
	}
	return true;
}

//-------------------------------------------------------------------------------------
int Update2DActorMessage::_lua_get_actor_id(lua_State *L)
{
	const Update2DActorMessage* msg = (const Update2DActorMessage*)lua_touserdata(L, 1);

	QString id = QString("%1").arg(msg->getActorID());
	lua_pushstring(L, id.toUtf8().toStdString().c_str());
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

//-------------------------------------------------------------------------------------
int Update2DActorMessage::_lua_get_actor_info(lua_State *L)
{
	const Update2DActorMessage* msg = (const Update2DActorMessage*)lua_touserdata(L, 1);

	QString actorInfo = msg->getActorInfo();

	QByteArray msgUtf8 = actorInfo.toUtf8();
	lua_pushstring(L, msgUtf8.data());
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
		{ "get_actor_info", Update2DActorMessage::_lua_get_actor_info },

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
End2DSceneMessage::End2DSceneMessage(SessionPtr session, const axtrace_time_s& traceTime)
	: Message(session, traceTime)
{

}

//--------------------------------------------------------------------------------------------
End2DSceneMessage::~End2DSceneMessage()
{

}

//--------------------------------------------------------------------------------------------
bool End2DSceneMessage::build(const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	if (!(m_session->isHandshaked())) return false;

	axtrace_2d_end_scene_s value_head;
	size_t len = ringBuf->memcpy_out(&value_head, sizeof(value_head));
	assert(len == sizeof(value_head));
	//copy name 
	char tempName[AXTRACE_MAX_SCENE_NAME_LENGTH] = { 0 };
	int name_length = value_head.name_len;
	if (name_length > AXTRACE_MAX_SCENE_NAME_LENGTH) name_length = AXTRACE_MAX_SCENE_NAME_LENGTH;
	if (name_length > 0) {
		len = ringBuf->memcpy_out(tempName, name_length);
		assert(len == name_length);
		tempName[name_length - 1] = 0; //make sure last char is '\0'
		m_sceneName = QString::fromUtf8(tempName);
	}
	return true;
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
