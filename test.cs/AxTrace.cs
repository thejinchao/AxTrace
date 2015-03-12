using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Net;
using System.Net.Sockets;

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

        public static readonly int AXV_INT8 = 0;
        public static readonly int AXV_UINT8 = 1;
        public static readonly int AXV_INT16 = 2;
        public static readonly int AXV_UINT16 = 3;
        public static readonly int AXV_INT32 = 4;
        public static readonly int AXV_UINT32 = 5;
        public static readonly int AXV_INT64 = 6;
        public static readonly int AXV_UINT64 = 7;
        public static readonly int AXV_FLOAT32 = 8;
        public static readonly int AXV_FLOAT64 = 9;
        public static readonly int AXV_STR_ACP = 10;
        public static readonly int AXV_STR_UTF8 = 11;
        public static readonly int AXV_STR_UTF16 = 12;
        
        public static readonly byte AXTRACE_CMD_TYPE_TRACE = 1;
        public static readonly byte AXTRACE_CMD_TYPE_VALUE = 2;

        public static readonly int AXTRACE_MAX_TRACE_STRING_LENGTH = 0x8000;

        //private static readonly ushort AX_CODEPAGE_ACP = 0;
        private static readonly ushort AX_CODEPAGE_UTF8 = 1;
        //private static readonly ushort AX_CODEPAGE_UTF16 = 2;

        /*---------------------------------------------------------------------------------------------*/
        /* AxTrace Global data  */
        private class Context
        {
            public bool inited;
            public bool init_success;
            public Socket socket;
            public string server_addr;
            public Int32 server_port;

            public Context()
            {
                inited = false;
                init_success = false;
                server_addr = "127.0.0.1";
                server_port = 1978;
            }
        };

        [ThreadStatic]
        static Context g_context = new Context();

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

        static private Context _getContext()
        {
            if (g_context.inited) return g_context;

            try
            {
                g_context.inited = true;

                //try connect to server
                IPEndPoint iep = new IPEndPoint(Dns.GetHostEntry(g_context.server_addr).AddressList[0], g_context.server_port);
                g_context.socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                g_context.socket.Connect(iep);

                g_context.init_success = true;
            }
            catch (Exception)
            {
                g_context.init_success = false;
            }
            return g_context;
        }

        static public void Trace(uint style, string format, params object[] args)
        {
            Context ctx = _getContext();
            if (!ctx.init_success) return;

            int headSize = Marshal.SizeOf(typeof(axtrace_trace_s));
            byte[] buf = new byte[headSize + AXTRACE_MAX_TRACE_STRING_LENGTH];

            string content = System.String.Format(format, args);
            int content_length = content.Length + 1;
            int final_length = Marshal.SizeOf(typeof(axtrace_trace_s)) + content_length;

            axtrace_trace_s head = new axtrace_trace_s();
            head.head.length = (ushort)(final_length);
            head.head.flag = (byte)'A';
            head.head.type = AXTRACE_CMD_TYPE_TRACE;
            head.head.pid = 0;
            head.head.tid = 0;
            head.head.style = style;
            head.code_page = AX_CODEPAGE_UTF8;
            head.length = (ushort)content_length;

            IntPtr headPtr = Marshal.AllocHGlobal(headSize);
            Marshal.StructureToPtr(head, headPtr, false);
            Marshal.Copy(headPtr, buf, 0, headSize);
            System.Text.Encoding.UTF8.GetBytes(content).CopyTo(buf, headSize);
            Marshal.FreeHGlobal(headPtr);

            ctx.socket.Send(buf, 0, final_length, SocketFlags.DontRoute);
        }

        static public void Value(string name, sbyte value)
        {
        }
        static public void Value(string name, byte value)
        {
        }
        static public void Value(string name, short value)
        {
        }
        static public void Value(string name, ushort value)
        {
        }
        static public void Value(string name, int value)
        {
        }
        static public void Value(string name, uint value)
        {
        }
        static public void Value(string name, long value)
        {
        }
        static public void Value(string name, ulong value)
        {
        }
        static public void Value(string name, float value)
        {
        }
        static public void Value(string name, double value)
        {
        }
        static public void Value(string name, string value)
        {
        }
    }

}
