/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
#include "stdafx.h"
#include "AT4_Message.h"

//--------------------------------------------------------------------------------------------
Message::Message(SessionPtr session, const MessageTime& traceTime)
	: m_session(session)
{
	memcpy(&m_time, &traceTime, sizeof(MessageTime));
}

//--------------------------------------------------------------------------------------------
Message::~Message()
{

}

//-------------------------------------------------------------------------------------
qint32 Message::getMessageMaxSize(qint32 msgType)
{
	static qint32 s_MessageMaxSize[] =
	{
		sizeof(axtrace_shakehand_s) + AXTRACE_MAX_PROCESSNAME_LENGTH,	// AXTRACE_CMD_TYPE_SHAKEHAND(0)
		sizeof(axtrace_log_s) + AXTRACE_MAX_LOG_STRING_LENGTH,			// AXTRACE_CMD_TYPE_LOG(1)
		sizeof(axtrace_value_s) + AXTRACE_MAX_VALUENAME_LENGTH + AXTRACE_MAX_VALUE_LENGTH,	//AXTRACE_CMD_TYPE_VALUE(2)
		sizeof(axtrace_2d_begin_scene_s) + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_SCENE_DEFINE_LENGTH, //AXTRACE_CMD_TYPE_2D_BEGIN_SCENE(3)
		sizeof(axtrace_2d_actor_s) + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_ACTOR_INFO_LENGTH, //AXTRACE_CMD_TYPE_2D_ACTOR(4)
		sizeof(axtrace_2d_end_scene_s) + AXTRACE_MAX_SCENE_NAME_LENGTH, //AXTRACE_CMD_TYPE_2D_END_SCENE(5)
		sizeof(axtrace_2d_actor_log_s) + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_ACTOR_LOG_LENGTH,	//AXTRACE_CMD_TYPE_2D_ACTOR_LOG
	};
	static qint32 s_MessageTypeCounts = sizeof(s_MessageMaxSize) / sizeof(s_MessageMaxSize[0]);

	if (msgType < 0 || msgType >= s_MessageTypeCounts) return -1;

	return s_MessageMaxSize[msgType];
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
ShakehandMessage::ShakehandMessage(SessionPtr session, const MessageTime& traceTime)
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

	//shakehand already
	if (!(m_session->onSessionShakehand(this))) return false;

	//ok!
	ringBuf->discard(head.length);
	return true;
}

//--------------------------------------------------------------------------------------------
QQueue<LogMessage*> LogMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
LogMessage::LogMessage(SessionPtr session, const MessageTime& traceTime)
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
	size_t len = ringBuf->peek(0, &logHead, sizeof(axtrace_log_s));
	assert(len == sizeof(axtrace_log_s));

	m_logType = logHead.log_type;

	//check log length
	qint32 logByteLength = logHead.length;
	if (logByteLength<0 || logByteLength>AXTRACE_MAX_LOG_STRING_LENGTH) return false;

	//check length
	if (head.length != sizeof(axtrace_log_s) + logByteLength) return false;

	QByteArray& cache = memoryCache.localData();
	if (cache.size() < logByteLength + 2)
		cache.resize(logByteLength + 2);

	len = ringBuf->peek(sizeof(axtrace_log_s), cache.data(), logByteLength);
	assert(len == logByteLength);

	cache.data()[logByteLength + 0] = 0;
	cache.data()[logByteLength + 1] = 0;

	switch (logHead.code_page)
	{
	case ATC_UTF16:
		m_log = QString::fromUtf16((const char16_t*)cache.data());
		break;
	case ATC_UTF8:
		m_log = QString::fromUtf8((const char*)cache);
		break;
	case ATC_ACP:
		m_log = QString::fromLocal8Bit((const char*)cache);
		break;
	}

	//ok!
	ringBuf->discard(head.length);
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
ValueMessage::ValueMessage(SessionPtr session, const MessageTime& traceTime)
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
	size_t len = ringBuf->peek(0, &value_head, sizeof(value_head));
	assert(len == sizeof(value_head));

	m_valueType = value_head.value_type;
	
	//check value length
	m_valueSize = value_head.value_len;
	if (m_valueSize<0 || m_valueSize>AXTRACE_MAX_VALUE_LENGTH) return false;

	//check name length
	qint32 name_length = value_head.name_len;
	if (name_length<=0 || name_length > AXTRACE_MAX_VALUENAME_LENGTH) return false;

	//check length
	if (head.length != sizeof(value_head) + name_length + m_valueSize) return false;

	//copy name 
	char tempName[AXTRACE_MAX_VALUENAME_LENGTH];
	len = ringBuf->peek(sizeof(axtrace_value_s), tempName, name_length);
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
	len = ringBuf->peek(sizeof(axtrace_value_s) + name_length, m_valueBuf, m_valueSize);
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

	//ok!
	ringBuf->discard(head.length);
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
		value = QString::fromUcs4((const char32_t*)m_valueBuf);
		break;

	case AXV_STR_UTF16:
		value = QString::fromUtf16((const char16_t*)m_valueBuf);
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
Begin2DSceneMessage::Begin2DSceneMessage(SessionPtr session, const MessageTime& traceTime)
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

	axtrace_2d_begin_scene_s begin_scene_head;
	size_t len = ringBuf->peek(0, &begin_scene_head, sizeof(begin_scene_head));
	assert(len == sizeof(begin_scene_head));

	m_sceneRect = QRectF(begin_scene_head.x_min, begin_scene_head.y_min, 
		begin_scene_head.x_max-begin_scene_head.x_min, begin_scene_head.y_max - begin_scene_head.y_min);

	//check scene name
	qint32 name_length = begin_scene_head.name_len;
	if (name_length <= 0 || name_length > AXTRACE_MAX_SCENE_NAME_LENGTH) return false;

	//check scene define
	qint32 define_length = begin_scene_head.define_len;
	if (define_length<0 || define_length > AXTRACE_MAX_SCENE_DEFINE_LENGTH) return false;

	//check length
	if (head.length != sizeof(begin_scene_head) + name_length + define_length) return false;

	//copy name 
	char tempBuf[AXTRACE_MAX_SCENE_DEFINE_LENGTH] = { 0 };
	len = ringBuf->peek(sizeof(begin_scene_head), tempBuf, name_length);
	assert(len == name_length);
	tempBuf[name_length - 1] = 0; //make sure last char is '\0'
	m_sceneName = QString::fromUtf8(tempBuf);

	//copy define 
	if (define_length > 0) {
		len = ringBuf->peek(sizeof(begin_scene_head)+ name_length, tempBuf, define_length);
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
	//ok!
	ringBuf->discard(head.length);
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
Update2DActorMessage::Update2DActorMessage(SessionPtr session, const MessageTime& traceTime)
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

	axtrace_2d_actor_s actor_head;
	size_t len = ringBuf->peek(0, &actor_head, sizeof(actor_head));
	assert(len == sizeof(actor_head));

	m_actorID = (qint64)actor_head.actor_id;
	m_position = QPointF((qreal)actor_head.x, (qreal)actor_head.y);
	m_dir = (qreal)actor_head.dir;
	m_style = (quint32)actor_head.style;

	//check scene name
	qint32 name_length = actor_head.name_len;
	if (name_length <= 0 || name_length > AXTRACE_MAX_SCENE_NAME_LENGTH) return false;

	//check actor info length
	qint32 info_length = actor_head.info_len;
	if (info_length<0 || info_length > AXTRACE_MAX_ACTOR_INFO_LENGTH) return false;

	//check length
	if (head.length != sizeof(actor_head) + name_length + info_length) return false;

	//copy name 
	char tempName[AXTRACE_MAX_SCENE_NAME_LENGTH] = { 0 };
	len = ringBuf->peek(sizeof(actor_head), tempName, name_length);
	assert(len == name_length);
	tempName[name_length - 1] = 0; //make sure last char is '\0'
	m_sceneName = QString::fromUtf8(tempName);
	
	//copy info
	char tempInfo[AXTRACE_MAX_ACTOR_INFO_LENGTH] = { 0 };
	if (info_length > 0) {
		len = ringBuf->peek(sizeof(actor_head)+name_length, tempInfo, info_length);
		assert(len == info_length);
		tempInfo[info_length - 1] = 0; //make sure last char is '\0'
		m_actorInfo = QString::fromUtf8(tempInfo);
	}
	else
	{
		m_actorInfo = QString();
	}

	//ok!
	ringBuf->discard(head.length);
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
End2DSceneMessage::End2DSceneMessage(SessionPtr session, const MessageTime& traceTime)
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

	axtrace_2d_end_scene_s end_scene_head;
	size_t len = ringBuf->peek(0, &end_scene_head, sizeof(end_scene_head));
	assert(len == sizeof(end_scene_head));

	//check scene name
	qint32 name_length = end_scene_head.name_len;
	if (name_length <= 0 || name_length > AXTRACE_MAX_SCENE_NAME_LENGTH) return false;

	//check length
	if (head.length != sizeof(end_scene_head) + name_length) return false;

	//copy name 
	char tempName[AXTRACE_MAX_SCENE_NAME_LENGTH] = { 0 };

	len = ringBuf->peek(sizeof(end_scene_head), tempName, name_length);
	assert(len == name_length);
	tempName[name_length - 1] = 0; //make sure last char is '\0'
	m_sceneName = QString::fromUtf8(tempName);

	//ok!
	ringBuf->discard(head.length);
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

//--------------------------------------------------------------------------------------------
QQueue<Add2DActorLogMessage*> Add2DActorLogMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
Add2DActorLogMessage::Add2DActorLogMessage(SessionPtr session, const MessageTime& traceTime)
	: Message(session, traceTime)
{

}

//--------------------------------------------------------------------------------------------
Add2DActorLogMessage::~Add2DActorLogMessage()
{

}

//--------------------------------------------------------------------------------------------
bool Add2DActorLogMessage::build(const axtrace_head_s& head, cyclone::RingBuf* ringBuf)
{
	if (!(m_session->isHandshaked())) return false;

	axtrace_2d_actor_log_s message_head;
	size_t len = ringBuf->peek(0, &message_head, sizeof(message_head));
	assert(len == sizeof(message_head));

	m_actorID = (qint64)message_head.actor_id;

	//check scene name
	qint32 name_length = message_head.name_len;
	if (name_length <= 0 || name_length > AXTRACE_MAX_SCENE_NAME_LENGTH) return false;

	//check log 
	qint32 log_length = message_head.log_len;
	if (log_length < 0 || log_length > AXTRACE_MAX_ACTOR_LOG_LENGTH) return false;

	//check length
	if (head.length != sizeof(message_head) + name_length + log_length) return false;

	//copy scene name 
	char tempName[AXTRACE_MAX_SCENE_NAME_LENGTH] = { 0 };
	len = ringBuf->peek(sizeof(message_head), tempName, name_length);
	assert(len == name_length);
	tempName[name_length - 1] = 0; //make sure last char is '\0'
	m_sceneName = QString::fromUtf8(tempName);

	//copy actor log
	char tempLog[AXTRACE_MAX_ACTOR_LOG_LENGTH] = { 0 };
	if (log_length > 0) {
		len = ringBuf->peek(sizeof(message_head)+name_length, tempLog, log_length);
		assert(len == log_length);
		tempLog[log_length - 1] = 0; //make sure last char is '\0'
		m_actorLog = QString::fromUtf8(tempLog);
	}

	//ok!
	ringBuf->discard(head.length);
	return true;
}

//-------------------------------------------------------------------------------------
int Add2DActorLogMessage::_lua_get_actor_id(lua_State *L)
{
	const Add2DActorLogMessage* msg = (const Add2DActorLogMessage*)lua_touserdata(L, 1);

	QString id = QString("%1").arg(msg->getActorID());
	lua_pushstring(L, id.toUtf8().toStdString().c_str());
	return 1;
}

//-------------------------------------------------------------------------------------
int Add2DActorLogMessage::_lua_get_actor_log(lua_State *L)
{
	const Add2DActorLogMessage* msg = (const Add2DActorLogMessage*)lua_touserdata(L, 1);

	QString actorLog = msg->getActorLog();

	QByteArray msgUtf8 = actorLog.toUtf8();
	lua_pushstring(L, msgUtf8.data());
	return 1;
}

//--------------------------------------------------------------------------------------------
const char* Add2DActorLogMessage::MetaName = "AxTrace.AddActor2DLog";

void Add2DActorLogMessage::_luaopen(lua_State *L)
{
	static luaL_Reg msg_data_meta[] =
	{
		{ "get_type", Message::_lua_get_type },
		{ "get_pid", Message::_lua_get_process_id },
		{ "get_tid", Message::_lua_get_thread_id },

		{ "get_actor_id", Add2DActorLogMessage::_lua_get_actor_id },
		{ "get_actor_log", Add2DActorLogMessage::_lua_get_actor_log },

		{ 0, 0 }
	};


	//PlayerData meta table
	luaL_newmetatable(L, Add2DActorLogMessage::MetaName);
	lua_pushvalue(L, -1);  /* push metatable */
	lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */

	luaL_register(L, NULL, msg_data_meta);  /* file methods */
}
