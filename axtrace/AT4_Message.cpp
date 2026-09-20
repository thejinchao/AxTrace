/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2026
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
qint32 Message::getMessageMinSize(qint32 msgType)
{
	static qint32 s_MessageMinSize[] =
	{
		sizeof(axtrace_shakehand_s),		// AXTRACE_CMD_TYPE_SHAKEHAND(0)
		sizeof(axtrace_log_s),				// AXTRACE_CMD_TYPE_LOG(1)
		sizeof(axtrace_value_s),			//AXTRACE_CMD_TYPE_VALUE(2)
		sizeof(axtrace_2d_begin_scene_s),	//AXTRACE_CMD_TYPE_2D_BEGIN_SCENE(3)
		sizeof(axtrace_2d_actor_s),			//AXTRACE_CMD_TYPE_2D_ACTOR(4)
		sizeof(axtrace_2d_end_scene_s),		//AXTRACE_CMD_TYPE_2D_END_SCENE(5)
		sizeof(axtrace_2d_actor_log_s),		//AXTRACE_CMD_TYPE_2D_ACTOR_LOG
	};
	static qint32 s_MessageTypeCounts = sizeof(s_MessageMinSize) / sizeof(s_MessageMinSize[0]);

	if (msgType < 0 || msgType >= s_MessageTypeCounts) return -1;

	return s_MessageMinSize[msgType];
}

//-------------------------------------------------------------------------------------
bool Message::readBytes(char* target, QByteArrayView& dataBuff, qint32 byteSize)
{
	if (byteSize > dataBuff.size()) return false;
	if (byteSize <= 0) return true;

	memcpy(target, dataBuff.data(), byteSize);
	dataBuff.slice(byteSize);
	return true;
}

//-------------------------------------------------------------------------------------
bool Message::readString_Utf8(QString& value, QByteArrayView& dataBuff, qint32 stringByteSize, qint32 maxStringByteSize)
{
	if (stringByteSize > maxStringByteSize || stringByteSize > dataBuff.size()) return false;
	if (stringByteSize <= 0)
	{
		value.clear();
		return true;
	}

	value = QString::fromUtf8((const char*)dataBuff.data(), stringByteSize - 1); //remove `\0'
	dataBuff.slice(stringByteSize);
	return true;
}

//-------------------------------------------------------------------------------------
bool Message::readString_Utf16(QString& value, QByteArrayView& dataBuff, qint32 stringByteSize, qint32 maxStringByteSize)
{
	if (stringByteSize > maxStringByteSize || stringByteSize > dataBuff.size()) return false;
	if (stringByteSize <= 0)
	{
		value.clear();
		return true;
	}

	value = QString::fromUtf16((const char16_t*)dataBuff.data(), stringByteSize / sizeof(wchar_t) - 1); //remove L`\0'
	dataBuff.slice(stringByteSize);
	return true;
}

//-------------------------------------------------------------------------------------
bool Message::readString_Local8Bit(QString& value, QByteArrayView& dataBuff, qint32 stringByteSize, qint32 maxStringByteSize)
{
	if (stringByteSize > maxStringByteSize || stringByteSize > dataBuff.size()) return false;
	if (stringByteSize <= 0)
	{
		value.clear();
		return true;
	}

	value = QString::fromLocal8Bit((const char*)dataBuff.data(), stringByteSize - 1); //remove `\0'
	dataBuff.slice(stringByteSize);
	return true;
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
	, m_version(0)
	, m_processID(0)
	, m_threadID(0)
{

}

//--------------------------------------------------------------------------------------------
ShakehandMessage::~ShakehandMessage()
{

}

//--------------------------------------------------------------------------------------------
bool ShakehandMessage::build(QByteArrayView data)
{
	axtrace_shakehand_s shakehand;
	if (!readBytes((char*)(&shakehand), data, sizeof(shakehand))) return false;

	//check version
	if (shakehand.ver != AXTRACE_PROTO_VERSION) return false;

	//check name length
	qint32 sessionNameLength = shakehand.sname_len;
	if (sessionNameLength <= 0 || sessionNameLength > AXTRACE_MAX_PROCESSNAME_LENGTH) return false;

	//read session name
	if (!readString_Utf8(m_sessionName, data, sessionNameLength, AXTRACE_MAX_PROCESSNAME_LENGTH)) return false;

	m_version = shakehand.ver;
	m_processID = shakehand.pid;
	m_threadID = shakehand.tid;

	//is shakehand already?
	if (!(m_session->onSessionShakehand(this))) return false;

	//ok!
	Q_ASSERT(data.isEmpty());
	return true;
}

//--------------------------------------------------------------------------------------------
QQueue<LogMessage*> LogMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
LogMessage::LogMessage(SessionPtr session, const MessageTime& traceTime)
	: Message(session, traceTime)
	, m_logType(0)
{

}

//--------------------------------------------------------------------------------------------
LogMessage::~LogMessage()
{

}

//--------------------------------------------------------------------------------------------
bool LogMessage::build(QByteArrayView data)
{
	size_t msgByteLength = data.size();

	axtrace_log_s logHead;
	if (!readBytes((char*)(&logHead), data, sizeof(logHead))) return false;

	m_logType = logHead.log_type;

	//check log length
	qint32 logByteLength = logHead.length;
	if (logByteLength != msgByteLength - sizeof(axtrace_log_s)) return false;

	switch (logHead.code_page)
	{
	case ATC_UTF16:
		if (!readString_Utf16(m_log, data, logByteLength, AXTRACE_MAX_LOG_STRING_LENGTH)) return false;
		break;
	case ATC_UTF8:
		if (!readString_Utf8(m_log, data, logByteLength, AXTRACE_MAX_LOG_STRING_LENGTH)) return false;
		break;
	case ATC_ACP:
		if (!readString_Local8Bit(m_log, data, logByteLength, AXTRACE_MAX_LOG_STRING_LENGTH)) return false;
		break;
	default:
		return false;
	}

	//ok!
	Q_ASSERT(data.isEmpty());
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

	luaL_setfuncs(L, msg_data_meta, 0);  /* file methods */
}

//--------------------------------------------------------------------------------------------
QQueue<ValueMessage*> ValueMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
ValueMessage::ValueMessage(SessionPtr session, const MessageTime& traceTime)
	: Message(session, traceTime)
	, m_valueType(0)
	, m_valueSize(0)
	, m_valueBuf(STANDARD_VALUE_SIZE, 0)
{
}

//--------------------------------------------------------------------------------------------
ValueMessage::~ValueMessage()
{

}

//--------------------------------------------------------------------------------------------
bool ValueMessage::build(QByteArrayView data)
{
	if (!(m_session->isHandshaked())) return false;

	axtrace_value_s valueHead;
	if (!readBytes((char*)(&valueHead), data, sizeof(valueHead))) return false;

	m_valueType = valueHead.value_type;

	//check value length
	m_valueSize = valueHead.value_len;
	if (m_valueSize<0 || m_valueSize>AXTRACE_MAX_VALUE_LENGTH) return false;

	//check name length
	qint32 nameLength = valueHead.name_len;
	if (nameLength <= 0 || nameLength > AXTRACE_MAX_VALUENAME_LENGTH) return false;

	//check length
	if (valueHead.head.length != sizeof(valueHead) + nameLength + m_valueSize) return false;

	//read value name 
	if (!readString_Utf8(m_name, data, nameLength, AXTRACE_MAX_VALUENAME_LENGTH)) return false;

	//value
	if (m_valueSize > STANDARD_VALUE_SIZE)
	{
		//big value
		m_valueBuf.resize(m_valueSize, 0);
	}

	if (!readBytes(m_valueBuf.data(), data, m_valueSize)) return false;

	//ok!
	Q_ASSERT(data.isEmpty());
	return true;
}

//--------------------------------------------------------------------------------------------
void ValueMessage::getValueAsString(QString& value) const
{
	const static QString ERROR_VALUE("<ERR>");

	switch (m_valueType)
	{
	case AXV_INT8:
		value = QString::number(*((int8_t*)m_valueBuf.data()));
		break;

	case AXV_UINT8:
		value = QString::number(*((uint8_t*)m_valueBuf.data()));
		break;

	case AXV_INT16:
		value = QString::number(*((int16_t*)m_valueBuf.data()));
		break;

	case AXV_UINT16:
		value = QString::number(*((uint16_t*)m_valueBuf.data()));
		break;

	case AXV_INT32:
		value = QString::number(*((int32_t*)m_valueBuf.data()));
		break;

	case AXV_UINT32:
		value = QString::number(*((uint32_t*)m_valueBuf.data()));
		break;

	case AXV_INT64:
		value = QString::number(*((int64_t*)m_valueBuf.data()));
		break;

	case AXV_UINT64:
		value = QString::number(*((uint64_t*)m_valueBuf.data()));
		break;

	case AXV_FLOAT32:
		value = QString::number(*((float*)m_valueBuf.data()));
		break;

	case AXV_FLOAT64:
		value = QString::number(*((double*)m_valueBuf.data()));
		break;

	case AXV_STR_UTF32:
		value = QString::fromUcs4((const char32_t*)m_valueBuf.data());
		break;

	case AXV_STR_UTF16:
		value = QString::fromUtf16((const char16_t*)m_valueBuf.data());
		break;

	case AXV_STR_UTF8:
		value = QString::fromUtf8((const char*)m_valueBuf.data());
		break;

	case AXV_STR_ACP:
		value = QString::fromLocal8Bit((const char*)m_valueBuf.data());
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

	luaL_setfuncs(L, msg_data_meta, 0);  /* file methods */
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
bool Begin2DSceneMessage::build(QByteArrayView data)
{
	if (!(m_session->isHandshaked())) return false;

	axtrace_2d_begin_scene_s beginSceneHead;
	if (!readBytes((char*)(&beginSceneHead), data, sizeof(beginSceneHead))) return false;

	m_sceneRect = QRectF(beginSceneHead.x_min, beginSceneHead.y_min,
		beginSceneHead.x_max - beginSceneHead.x_min, beginSceneHead.y_max - beginSceneHead.y_min);

	//check scene name
	qint32 nameLength = beginSceneHead.name_len;
	if (nameLength <= 0 || nameLength > AXTRACE_MAX_SCENE_NAME_LENGTH) return false;

	//check scene define
	qint32 defineLength = beginSceneHead.define_len;
	if (defineLength<0 || defineLength > AXTRACE_MAX_SCENE_DEFINE_LENGTH) return false;

	//check length
	if (beginSceneHead.head.length != sizeof(beginSceneHead) + nameLength + defineLength) return false;

	//read scene name 
	if (!readString_Utf8(m_sceneName, data, nameLength, AXTRACE_MAX_SCENE_NAME_LENGTH)) return false;

	//parser scene define 
	if (defineLength > 0)
	{
		//make json object
		QJsonParseError jerror;
		QJsonDocument jsonDocument = QJsonDocument::fromJson(data.data(), &jerror);
		if (jerror.error == QJsonParseError::NoError)
		{
			m_sceneDefine = jsonDocument.object();
		}
		data.slice(defineLength);
	}

	//ok!
	Q_ASSERT(data.isEmpty());
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

	luaL_setfuncs(L, msg_data_meta, 0);  /* file methods */
}

//--------------------------------------------------------------------------------------------
QQueue<Update2DActorMessage*> Update2DActorMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
Update2DActorMessage::Update2DActorMessage(SessionPtr session, const MessageTime& traceTime)
	: Message(session, traceTime)
	, m_actorID(0)
	, m_dir(0)
	, m_actorStyle(0)
{

}

//--------------------------------------------------------------------------------------------
Update2DActorMessage::~Update2DActorMessage()
{

}

//--------------------------------------------------------------------------------------------
bool Update2DActorMessage::build(QByteArrayView data)
{
	if (!(m_session->isHandshaked())) return false;

	axtrace_2d_actor_s actorHead;
	if (!readBytes((char*)(&actorHead), data, sizeof(actorHead))) return false;

	m_actorID = (qint64)actorHead.actor_id;
	m_position = QPointF((qreal)actorHead.x, (qreal)actorHead.y);
	m_dir = (qreal)actorHead.dir;
	m_actorStyle = (quint32)actorHead.style;

	//check scene name
	qint32 sceneNameLength = actorHead.name_len;
	if (sceneNameLength <= 0 || sceneNameLength > AXTRACE_MAX_SCENE_NAME_LENGTH) return false;

	//check actor info length
	qint32 actorInfoLength = actorHead.info_len;
	if (actorInfoLength<0 || actorInfoLength > AXTRACE_MAX_ACTOR_INFO_LENGTH) return false;

	//check length
	if (actorHead.head.length != sizeof(actorHead) + sceneNameLength + actorInfoLength) return false;

	//read scene name 
	if (!readString_Utf8(m_sceneName, data, sceneNameLength, AXTRACE_MAX_SCENE_NAME_LENGTH)) return false;

	//copy info
	if (actorInfoLength > 0)
	{
		if (!readString_Utf8(m_actorInfo, data, actorInfoLength, AXTRACE_MAX_ACTOR_INFO_LENGTH)) return false;
	}
	else
	{
		m_actorInfo = QString();
	}

	//ok!
	Q_ASSERT(data.isEmpty());
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

	luaL_setfuncs(L, msg_data_meta, 0);  /* file methods */
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
bool End2DSceneMessage::build(QByteArrayView data)
{
	if (!(m_session->isHandshaked())) return false;

	axtrace_2d_end_scene_s endSceneHead;
	if (!readBytes((char*)(&endSceneHead), data, sizeof(endSceneHead))) return false;

	//check scene name
	qint32 sceneNameLength = endSceneHead.name_len;
	if (sceneNameLength <= 0 || sceneNameLength > AXTRACE_MAX_SCENE_NAME_LENGTH) return false;

	//check length
	if (endSceneHead.head.length != sizeof(endSceneHead) + sceneNameLength) return false;

	//read scene name 
	if (!readString_Utf8(m_sceneName, data, sceneNameLength, AXTRACE_MAX_SCENE_NAME_LENGTH)) return false;

	//ok!
	Q_ASSERT(data.isEmpty());
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

	luaL_setfuncs(L, msg_data_meta, 0);  /* file methods */
}

//--------------------------------------------------------------------------------------------
QQueue<Add2DActorLogMessage*> Add2DActorLogMessage::s_messagePool;

//--------------------------------------------------------------------------------------------
Add2DActorLogMessage::Add2DActorLogMessage(SessionPtr session, const MessageTime& traceTime)
	: Message(session, traceTime)
	, m_actorID(0)
{

}

//--------------------------------------------------------------------------------------------
Add2DActorLogMessage::~Add2DActorLogMessage()
{

}

//--------------------------------------------------------------------------------------------
bool Add2DActorLogMessage::build(QByteArrayView data)
{
	if (!(m_session->isHandshaked())) return false;

	axtrace_2d_actor_log_s actorLogHead;
	if (!readBytes((char*)(&actorLogHead), data, sizeof(actorLogHead))) return false;

	m_actorID = (qint64)actorLogHead.actor_id;

	//check scene name
	qint32 sceneNameLength = actorLogHead.name_len;
	if (sceneNameLength <= 0 || sceneNameLength > AXTRACE_MAX_SCENE_NAME_LENGTH) return false;

	//check log 
	qint32 logLength = actorLogHead.log_len;
	if (logLength < 0 || logLength > AXTRACE_MAX_ACTOR_LOG_LENGTH) return false;

	//check length
	if (actorLogHead.head.length != sizeof(actorLogHead) + sceneNameLength + logLength) return false;

	//read scene name 
	if (!readString_Utf8(m_sceneName, data, sceneNameLength, AXTRACE_MAX_SCENE_NAME_LENGTH)) return false;

	//copy actor log
	if (logLength > 0)
	{
		if (!readString_Utf8(m_actorLog, data, logLength, AXTRACE_MAX_ACTOR_LOG_LENGTH)) return false;
	}

	//ok!
	Q_ASSERT(data.isEmpty());
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

	luaL_setfuncs(L, msg_data_meta, 0);  /* file methods */
}
