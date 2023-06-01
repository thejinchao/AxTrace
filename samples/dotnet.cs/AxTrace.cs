/***************************************************

				AXIA|Trace4

	(C) Copyright thecodeway.com 2023
***************************************************/
using System;
using System.Runtime.InteropServices;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Diagnostics;

namespace com.thecodeway
{
	public class AxTrace
	{
		/*---------------------------------------------------------------------------------------------*/
		public static readonly uint AXT_TRACE = 0;
		public static readonly uint AXT_DEBUG = 1;
		public static readonly uint AXT_INFO = 2;
		public static readonly uint AXT_WARN = 3;
		public static readonly uint AXT_ERROR = 4;
		public static readonly uint AXT_FATAL = 5;
		public static readonly uint AXT_USERDEF = 10;

		public static readonly uint AXV_INT8 = 0;
		public static readonly uint AXV_UINT8 = 1;
		public static readonly uint AXV_INT16 = 2;
		public static readonly uint AXV_UINT16 = 3;
		public static readonly uint AXV_INT32 = 4;
		public static readonly uint AXV_UINT32 = 5;
		public static readonly uint AXV_INT64 = 6;
		public static readonly uint AXV_UINT64 = 7;
		public static readonly uint AXV_FLOAT32 = 8;
		public static readonly uint AXV_FLOAT64 = 9;
		public static readonly uint AXV_STR_ACP = 10;
		public static readonly uint AXV_STR_UTF8 = 11;
		public static readonly uint AXV_STR_UTF16 = 12;
		public static readonly uint AXV_USER_DEF = 100;

		public static readonly byte AXTRACE_CMD_TYPE_SHAKEHAND = 0;
		public static readonly byte AXTRACE_CMD_TYPE_LOG = 1;
		public static readonly byte AXTRACE_CMD_TYPE_VALUE = 2;
		public static readonly byte AXTRACE_CMD_TYPE_2D_BEGIN_SCENE = 3;
		public static readonly byte AXTRACE_CMD_TYPE_2D_ACTOR = 4;
		public static readonly byte AXTRACE_CMD_TYPE_2D_END_SCENE = 5;
		public static readonly byte AXTRACE_CMD_TYPE_2D_ACTOR_LOG = 6;

		public static readonly int AXTRACE_MAX_TRACE_STRING_LENGTH = 0x8000;
		public static readonly int AXTRACE_MAX_VALUENAME_LENGTH = 128;
		public static readonly int AXTRACE_MAX_VALUE_LENGTH = 1024;
		public static readonly int AXTRACE_MAX_PROCESSNAME_LENGTH = 512;
		public static readonly int AXTRACE_MAX_SCENE_NAME_LENGTH = 128;
		public static readonly int AXTRACE_MAX_SCENE_DEFINE_LENGTH = 2048;
		public static readonly int AXTRACE_MAX_ACTOR_INFO_LENGTH = 2048;
		public static readonly int AXTRACE_MAX_ACTOR_LOG_LENGTH = 2048;

		public static readonly int AXTRACE_PROTO_VERSION = 4;

		//private static readonly ushort AX_CODEPAGE_ACP = 0;
		private static readonly ushort AX_CODEPAGE_UTF8 = 1;
		//private static readonly ushort AX_CODEPAGE_UTF16 = 2;

		private static string server_addr = "127.0.0.1";
		private static Int32 server_port = 1978;

		/*---------------------------------------------------------------------------------------------*/
		/* AxTrace Global data  */
		private class axtrace_context_s
		{
			public bool init_success = false;
			public Socket socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
		};

		[ThreadStatic]
		static axtrace_context_s? g_context = null;

		/*---------------------------------------------------------------------------------------------*/
		/* axtrace communication data struct*/
		[Serializable]
		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		private struct axtrace_head_s
		{
			public ushort length;   /* length */
			public byte flag;       /* magic flag, always 'A' */
			public byte type;       /* command type AXTRACE_CMD_TYPE_* */
		};

		[Serializable]
		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		private struct axtrace_shakehand_s
		{
			public axtrace_head_s head; /* common head */
			public ushort ver;          /* proto ver*/
			public ushort sname_len;    /* length of session name*/
			public uint pid;            /* process id*/
			public uint tid;            /* thread id*/
			/* [session name buf  with '\0' ended]*/
		};

		[Serializable]
		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		private struct axtrace_log_s
		{
			public axtrace_head_s head; /* common head */
			public uint log_type;       /* trace style AXT_* */
			public ushort code_page;    /* code page */
			public ushort length;       /* trace string length */
			/* [trace string data with '\0' ended] */
		};

		[Serializable]
		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		private struct axtrace_value_s
		{
			public axtrace_head_s head;     /* common head */
			public uint value_type;         /* value type AXV_* */
			public ushort name_len;         /* length of value name */
			public ushort value_len;        /* length of value */
			                                /* [name buf  with '\0' ended]*/
			                                /* [value buf] */
		};

		[Serializable]
		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		private struct axtrace_2d_begin_scene_s
		{
			public axtrace_head_s head;     /* common head */
			public double x_min;            /* left of scene*/
			public double y_min;            /* top of scene*/
			public double x_max;            /* right of scene*/
			public double y_max;            /* bottom of scene*/
			public ushort name_len;         /* length of scene name */
			public ushort define_len;       /* length of scene define */
			                                /* [scene name buf  with '\0' ended]*/
			                                /* [scene define buf  with '\0' ended]*/
		};

		[Serializable]
		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		private struct axtrace_2d_actor_s
		{
			public axtrace_head_s head;     /* common head */
			public Int64 actor_id;          /* id of actor */
			public double x;                /* position (x)*/
			public double y;                /* position (y)*/
			public double dir;              /* direction */
			public uint style;              /* user define style */
			public ushort name_len;         /* length of actor name */
			public ushort info_len;         /* length of actor information */
			                                /* [actor name buf  with '\0' ended]*/
			                                /* [actor information buf  with '\0' ended]*/
		};

		[Serializable]
		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		private struct axtrace_2d_end_scene_s
		{
			public axtrace_head_s head;     /* common head */
			public ushort name_len;         /* length of scene name */
			                                /* [scene name buf  with '\0' ended]*/
		};

		[Serializable]
		[StructLayout(LayoutKind.Sequential, Pack = 1)]
		struct axtrace_2d_actor_log_s
		{
			public axtrace_head_s head;     /* common head */
			public Int64 actor_id;          /* id of actor */
			public ushort name_len;         /* length of scene name */
			public ushort log_len;          /* length of actor log */
			                                /* [scene name buf  with '\0' ended]*/
			                                /* [actor log(utf8) buf  with '\0' ended]*/
		};

		/*---------------------------------------------------------------------------------------------*/
		static private axtrace_context_s? _getContext()
		{
			if (g_context != null) return g_context;

			try
			{
				g_context = new axtrace_context_s();

				//try connect to server
				IPEndPoint endPoint = new IPEndPoint(IPAddress.Parse(server_addr), server_port);
				g_context.socket.Connect(endPoint);

				//send shakehand message
				_sendHandShakeMessage(g_context);

				g_context.init_success = true;
			}
			catch (Exception)
			{
				if (g_context != null)
				{
					g_context.init_success = false;
				}
			}
			return g_context;
		}
		static private void _sendHandShakeMessage(axtrace_context_s ctx)
		{
			int headSize = Marshal.SizeOf(typeof(axtrace_shakehand_s));
			byte[] buf = new byte[headSize + AXTRACE_MAX_PROCESSNAME_LENGTH];

			//get current process name
			string currentAppName = Process.GetCurrentProcess().ProcessName;
			int pname_length = currentAppName.Length + 1;

			//calc final length
			int final_length = headSize + pname_length;

			axtrace_shakehand_s shakehand = new axtrace_shakehand_s();
			shakehand.head.length = (ushort)(final_length);
			shakehand.head.flag = (byte)'A';
			shakehand.head.type = AXTRACE_CMD_TYPE_SHAKEHAND;

			shakehand.ver = (ushort)(AXTRACE_PROTO_VERSION);
			shakehand.sname_len = (ushort)(pname_length);
			shakehand.pid = (uint)Process.GetCurrentProcess().Id;
			shakehand.tid = (uint)Thread.CurrentThread.ManagedThreadId;

			IntPtr headPtr = Marshal.AllocHGlobal(headSize);
			Marshal.StructureToPtr(shakehand, headPtr, false);
			Marshal.Copy(headPtr, buf, 0, headSize);
			System.Text.Encoding.UTF8.GetBytes(currentAppName).CopyTo(buf, headSize);
			buf[final_length - 1] = 0; //add '\0'
			Marshal.FreeHGlobal(headPtr);

			ctx.socket.Send(buf, 0, final_length, SocketFlags.DontRoute);
		}
		static public void SetServer(string ip, int port)
		{
			server_addr = ip;
			server_port = port;
		}

		static public void Log(uint style, string format, params object[] args)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			int headSize = Marshal.SizeOf(typeof(axtrace_log_s));
			byte[] buf = new byte[headSize + AXTRACE_MAX_TRACE_STRING_LENGTH];

			string content = System.String.Format(format, args);
			byte[] contentBytes = System.Text.Encoding.UTF8.GetBytes(content);

			//add '\0'
			int content_length = contentBytes.Length + 1;
			int final_length = Marshal.SizeOf(typeof(axtrace_log_s)) + content_length;

			axtrace_log_s head = new axtrace_log_s();
			head.head.length = (ushort)(final_length);
			head.head.flag = (byte)'A';
			head.head.type = AXTRACE_CMD_TYPE_LOG;

			head.log_type = style;
			head.code_page = AX_CODEPAGE_UTF8;
			head.length = (ushort)content_length;

			IntPtr headPtr = Marshal.AllocHGlobal(headSize);
			Marshal.StructureToPtr(head, headPtr, false);
			Marshal.Copy(headPtr, buf, 0, headSize);
			contentBytes.CopyTo(buf, headSize);
			buf[final_length - 1] = 0; //add '\0'
			Marshal.FreeHGlobal(headPtr);

			ctx.socket.Send(buf, 0, final_length, SocketFlags.DontRoute);
		}

		static private void _value(axtrace_context_s ctx, uint style, string name, uint value_type, byte[] valueBytes)
		{
			byte[] nameBytes = System.Text.Encoding.UTF8.GetBytes(name);
			int name_length = nameBytes.Length + 1; // add '\0'
			int value_length = valueBytes.Length;

			int headSize = Marshal.SizeOf(typeof(axtrace_value_s));
			byte[] buf = new byte[headSize + AXTRACE_MAX_VALUENAME_LENGTH + AXTRACE_MAX_VALUE_LENGTH];
			int final_length = Marshal.SizeOf(typeof(axtrace_value_s)) + name_length + value_length;

			axtrace_value_s head = new axtrace_value_s();
			head.head.length = (ushort)(final_length);
			head.head.flag = (byte)'A';
			head.head.type = AXTRACE_CMD_TYPE_VALUE;

			head.value_type = value_type;
			head.name_len = (ushort)(name_length);
			head.value_len = (ushort)(value_length);

			IntPtr headPtr = Marshal.AllocHGlobal(headSize);
			Marshal.StructureToPtr(head, headPtr, false);
			Marshal.Copy(headPtr, buf, 0, headSize);
			nameBytes.CopyTo(buf, headSize);
			buf[headSize + nameBytes.Length] = 0; //add '\0'
			valueBytes.CopyTo(buf, headSize + head.name_len);
			Marshal.FreeHGlobal(headPtr);

			ctx.socket.Send(buf, 0, final_length, SocketFlags.DontRoute);
		}
		static public void Value(uint style, string name, sbyte value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;
			byte[] valueBytes = new byte[1];
			valueBytes[0] = (byte)value;
			
			_value(ctx, style, name, AXV_UINT8, valueBytes);
		}
		static public void Value(uint style, string name, byte value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;
			byte[] valueBytes = new byte[1];
			valueBytes[0] = value;

			_value(ctx, style, name, AXV_INT8, valueBytes);
		}
		static public void Value(uint style, string name, short value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			_value(ctx, style, name,
				AXV_INT16,
				BitConverter.GetBytes(value));
		}
		static public void Value(uint style, string name, ushort value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			_value(ctx, style, name,
				AXV_UINT16,
				BitConverter.GetBytes(value));
		}
		static public void Value(uint style, string name, int value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			_value(ctx, style, name,
				AXV_INT32,
				BitConverter.GetBytes(value));
		}
		static public void Value(uint style, string name, uint value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;
			_value(ctx, style, name,
				AXV_UINT32,
				BitConverter.GetBytes(value));
		}
		static public void Value(uint style, string name, long value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			_value(ctx, style, name,
				AXV_INT64,
				BitConverter.GetBytes(value));
		}
		static public void Value(uint style, string name, ulong value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			_value(ctx, style, name,
				AXV_UINT64,
				BitConverter.GetBytes(value));
		}
		static public void Value(uint style, string name, float value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			_value(ctx, style, name,
				AXV_FLOAT32,
				BitConverter.GetBytes(value));
		}
		static public void Value(uint style, string name, double value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			_value(ctx, style, name,
				AXV_FLOAT64,
				BitConverter.GetBytes(value));
		}
		static public void Value(uint style, string name, string value)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			byte[] stringBytes = System.Text.Encoding.UTF8.GetBytes(value);
			byte[] withNull = new byte[stringBytes.Length + 1];
			stringBytes.CopyTo(withNull, 0);
			withNull[stringBytes.Length] = 0;
			_value(ctx, style, name,
				AXV_STR_UTF8,
				withNull);
		}

		static public void Scene2DBegin(string sceneName, double xMin, double yMin, double xMax, double yMax, string sceneDefine)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			int headSize = Marshal.SizeOf(typeof(axtrace_2d_begin_scene_s));
			byte[] buf = new byte[headSize + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_SCENE_DEFINE_LENGTH];

			byte[] sceneNameBytes = System.Text.Encoding.UTF8.GetBytes(sceneName);
			int sceneNameLength = sceneNameBytes.Length + 1;

			byte[] sceneDefineBytes = System.Text.Encoding.UTF8.GetBytes(sceneDefine);
			int sceneDefineLength = sceneDefineBytes.Length + 1;

			int final_length = headSize + sceneNameLength + sceneDefineLength;

			axtrace_2d_begin_scene_s head = new axtrace_2d_begin_scene_s();
			head.head.length = (ushort)(final_length);
			head.head.flag = (byte)'A';
			head.head.type = AXTRACE_CMD_TYPE_2D_BEGIN_SCENE;

			head.x_min = xMin;
			head.y_min = yMin;
			head.x_max = xMax;
			head.y_max = yMax;
			head.name_len = (ushort)sceneNameLength;
			head.define_len = (ushort)sceneDefineLength;

			IntPtr headPtr = Marshal.AllocHGlobal(headSize);
			Marshal.StructureToPtr(head, headPtr, false);
			Marshal.Copy(headPtr, buf, 0, headSize);

			sceneNameBytes.CopyTo(buf, headSize);
			buf[headSize + sceneNameBytes.Length] = 0; //add '\0'

			if (sceneDefine.Length > 0)
			{
				sceneDefineBytes.CopyTo(buf, headSize + sceneNameLength);
				buf[headSize + sceneNameBytes.Length + sceneDefineBytes.Length + 1] = 0; //add '\0'
			}
			Marshal.FreeHGlobal(headPtr);

			ctx.socket.Send(buf, 0, final_length, SocketFlags.DontRoute);
		}
		static public void Scene2DActor(string sceneName, Int64 actorId, double x, double y, double dir, uint actorStyle, string actorInfo)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			int headSize = Marshal.SizeOf(typeof(axtrace_2d_actor_s));
			byte[] buf = new byte[headSize + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_ACTOR_INFO_LENGTH];

			byte[] sceneNameBytes = System.Text.Encoding.UTF8.GetBytes(sceneName);
			int sceneNameLength = sceneNameBytes.Length + 1;

			byte[]? actorInfoBytes = actorInfo!=null ? System.Text.Encoding.UTF8.GetBytes(actorInfo) : null;
			int actorInfoLength = actorInfoBytes != null ? actorInfoBytes.Length + 1 : 0;

			int final_length = headSize + sceneNameLength + actorInfoLength;

			axtrace_2d_actor_s head = new axtrace_2d_actor_s();
			head.head.length = (ushort)(final_length);
			head.head.flag = (byte)'A';
			head.head.type = AXTRACE_CMD_TYPE_2D_ACTOR;

			head.actor_id = actorId;
			head.x = x;
			head.y = y;
			head.dir = dir;
			head.style = actorStyle;
			head.name_len = (ushort)sceneNameLength;
			head.info_len = (ushort)actorInfoLength;

			IntPtr headPtr = Marshal.AllocHGlobal(headSize);
			Marshal.StructureToPtr(head, headPtr, false);
			Marshal.Copy(headPtr, buf, 0, headSize);

			sceneNameBytes.CopyTo(buf, headSize);
			buf[headSize + sceneNameBytes.Length] = 0; //add '\0'

			if (actorInfoBytes != null && actorInfoBytes.Length > 0)
			{
				actorInfoBytes.CopyTo(buf, headSize + sceneNameLength);
				buf[headSize + sceneNameBytes.Length + actorInfoBytes.Length + 1] = 0; //add '\0'
			}
			Marshal.FreeHGlobal(headPtr);

			ctx.socket.Send(buf, 0, final_length, SocketFlags.DontRoute);
		}

		static public void Scene2DEnd(string sceneName)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			int headSize = Marshal.SizeOf(typeof(axtrace_2d_end_scene_s));
			byte[] buf = new byte[headSize + AXTRACE_MAX_SCENE_NAME_LENGTH];

			byte[] sceneNameBytes = System.Text.Encoding.UTF8.GetBytes(sceneName);
			int sceneNameLength = sceneNameBytes.Length + 1;

			int final_length = headSize + sceneNameLength;

			axtrace_2d_end_scene_s head = new axtrace_2d_end_scene_s();
			head.head.length = (ushort)(final_length);
			head.head.flag = (byte)'A';
			head.head.type = AXTRACE_CMD_TYPE_2D_END_SCENE;

			head.name_len = (ushort)sceneNameLength;

			IntPtr headPtr = Marshal.AllocHGlobal(headSize);
			Marshal.StructureToPtr(head, headPtr, false);
			Marshal.Copy(headPtr, buf, 0, headSize);

			sceneNameBytes.CopyTo(buf, headSize);
			buf[headSize + sceneNameBytes.Length] = 0; //add '\0'
			Marshal.FreeHGlobal(headPtr);

			ctx.socket.Send(buf, 0, final_length, SocketFlags.DontRoute);
		}

		static public void Scene2DActorLog(string sceneName, Int64 actorId, string actorLog)
		{
			axtrace_context_s? ctx = _getContext();
			if (ctx == null || !ctx.init_success) return;

			int headSize = Marshal.SizeOf(typeof(axtrace_2d_actor_log_s));
			byte[] buf = new byte[headSize + AXTRACE_MAX_SCENE_NAME_LENGTH + AXTRACE_MAX_ACTOR_INFO_LENGTH];

			byte[] sceneNameBytes = System.Text.Encoding.UTF8.GetBytes(sceneName);
			int sceneNameLength = sceneNameBytes.Length + 1;

			byte[] actorLogBytes = System.Text.Encoding.UTF8.GetBytes(actorLog);
			int actorLogLength = actorLogBytes.Length + 1;

			int final_length = headSize + sceneNameLength + actorLogLength;

			axtrace_2d_actor_log_s head = new axtrace_2d_actor_log_s();
			head.head.length = (ushort)(final_length);
			head.head.flag = (byte)'A';
			head.head.type = AXTRACE_CMD_TYPE_2D_ACTOR_LOG;

			head.actor_id = actorId;
			head.name_len = (ushort)sceneNameLength;
			head.log_len = (ushort)actorLogLength;

			IntPtr headPtr = Marshal.AllocHGlobal(headSize);
			Marshal.StructureToPtr(head, headPtr, false);
			Marshal.Copy(headPtr, buf, 0, headSize);

			sceneNameBytes.CopyTo(buf, headSize);
			buf[headSize + sceneNameBytes.Length] = 0; //add '\0'

			actorLogBytes.CopyTo(buf, headSize + sceneNameLength);
			buf[headSize + sceneNameBytes.Length + actorLogBytes.Length + 1] = 0; //add '\0'

			Marshal.FreeHGlobal(headPtr);

			ctx.socket.Send(buf, 0, final_length, SocketFlags.DontRoute);
		}
	}
}
