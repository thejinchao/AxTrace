using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
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
        
        public static readonly byte AXTRACE_CMD_TYPE_LOG = 1;
        public static readonly byte AXTRACE_CMD_TYPE_VALUE = 2;

        public static readonly int AXTRACE_MAX_TRACE_STRING_LENGTH = 0x8000;
        public static readonly int AXTRACE_MAX_VALUENAME_LENGTH	= 128;
        public static readonly int AXTRACE_MAX_VALUE_LENGTH	= 1024;

        //private static readonly ushort AX_CODEPAGE_ACP = 0;
        private static readonly ushort AX_CODEPAGE_UTF8 = 1;
        //private static readonly ushort AX_CODEPAGE_UTF16 = 2;

		private static string server_addr = "127.0.0.1";
		private static Int32 server_port = 1978;

        /*---------------------------------------------------------------------------------------------*/
        /* AxTrace Global data  */
        private class Context
        {
            public bool init_success;
            public Socket socket;

            public Context()
            {
                init_success = false;
            }
        };

        [ThreadStatic]
        static Context g_context = null;

        /*---------------------------------------------------------------------------------------------*/
        /* axtrace communication data struct*/
        [Serializable]
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct axtrace_head_s
        {
            public ushort length;   /* length */
            public byte flag;       /* magic flag, always 'A' */
            public byte type;       /* command type AXTRACE_CMD_TYPE_* */
            public uint pid;		/* process id*/
            public uint tid;		/* thread id*/
            public uint style;		/* trace style AXT_* */
        };

        [Serializable]
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct axtrace_trace_s
        {
            public axtrace_head_s head; /* common head */
            public ushort code_page;	/* code page */
            public ushort length;		/* trace string length */
        };

        [Serializable]
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct axtrace_value_s
        {
	        public axtrace_head_s	head;	/* common head */
	        public uint	value_type;		    /* value type AXV_* */
	        public ushort name_len;		    /* length of value name */
            public ushort value_len;		/* length of value */
        } ;

        static private Context _getContext()
        {
            if (g_context !=null) return g_context;

            try
            {
                g_context = new Context();
                
                //try connect to server
                g_context.socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                IPEndPoint iep = new IPEndPoint(IPAddress.Parse(server_addr), server_port);
                g_context.socket.Connect(iep);

                g_context.init_success = true;
            }
            catch (Exception)
            {
                g_context.init_success = false;
            }
            return g_context;
        }

        static public void SetServer(string ip, int port)
        {
			server_addr = ip;
			server_port = port;
        }
        
        static public void Log(uint style, string format, params object[] args)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            int headSize = Marshal.SizeOf(typeof(axtrace_trace_s));
            byte[] buf = new byte[headSize + AXTRACE_MAX_TRACE_STRING_LENGTH];

            string content = System.String.Format(format, args);
            //add '\0'
            int content_length = content.Length + 1;
            int final_length = Marshal.SizeOf(typeof(axtrace_trace_s)) + content_length;

            axtrace_trace_s head = new axtrace_trace_s();
            head.head.length = (ushort)(final_length);
            head.head.flag = (byte)'A';
            head.head.type = AXTRACE_CMD_TYPE_LOG;
            head.head.pid = (uint)Process.GetCurrentProcess().Id;
            head.head.tid = (uint)Thread.CurrentThread.ManagedThreadId;
            head.head.style = style;
            head.code_page = AX_CODEPAGE_UTF8;
            head.length = (ushort)content_length;

            IntPtr headPtr = Marshal.AllocHGlobal(headSize);
            Marshal.StructureToPtr(head, headPtr, false);
            Marshal.Copy(headPtr, buf, 0, headSize);
            System.Text.Encoding.UTF8.GetBytes(content).CopyTo(buf, headSize);
            buf[final_length - 1] = 0; //add '\0'
            Marshal.FreeHGlobal(headPtr);

            ctx.socket.Send(buf, 0, final_length, SocketFlags.DontRoute);
        }

        static private void _value(Context ctx, uint style, string name, uint value_type, byte[] valueBytes)
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
            head.head.pid = (uint)Process.GetCurrentProcess().Id;
            head.head.tid = (uint)Thread.CurrentThread.ManagedThreadId;
            head.head.style = style;

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
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            _value(ctx, style, name, 
                AXV_UINT8, 
                BitConverter.GetBytes(value));
        }
        static public void Value(uint style, string name, byte value)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            _value(ctx, style, name,
                AXV_INT8,
                BitConverter.GetBytes(value));
        }
        static public void Value(uint style, string name, short value)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            _value(ctx, style, name,
                AXV_INT16,
                BitConverter.GetBytes(value));
        }
        static public void Value(uint style, string name, ushort value)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            _value(ctx, style, name,
                AXV_UINT16,
                BitConverter.GetBytes(value));
        }
        static public void Value(uint style, string name, int value)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            _value(ctx, style, name,
                AXV_INT32,
                BitConverter.GetBytes(value));
        }
        static public void Value(uint style, string name, uint value)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            _value(ctx, style, name,
                AXV_UINT32,
                BitConverter.GetBytes(value));
        }
        static public void Value(uint style, string name, long value)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            _value(ctx, style, name,
                AXV_INT64,
                BitConverter.GetBytes(value));
        }
        static public void Value(uint style, string name, ulong value)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            _value(ctx, style, name,
                AXV_UINT64,
                BitConverter.GetBytes(value));
        }
        static public void Value(uint style, string name, float value)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            _value(ctx, style, name,
                AXV_FLOAT32,
                BitConverter.GetBytes(value));
        }
        static public void Value(uint style, string name, double value)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            _value(ctx, style, name,
                AXV_FLOAT64,
                BitConverter.GetBytes(value));
        }
        static public void Value(uint style, string name, string value)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            byte[] stringBytes = System.Text.Encoding.UTF8.GetBytes(value);
            byte[] withNull = new byte[stringBytes.Length+1];
            stringBytes.CopyTo(withNull, 0);
            withNull[stringBytes.Length] = 0;
            _value(ctx, style, name,
                AXV_STR_UTF8,
                withNull);
        }
    }

}
