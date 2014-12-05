using UnityEngine;
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Collections;

namespace com.thecodeway
{
    public class AxTrace
    {
        public static readonly int AX_INT8 = 0;
        public static readonly int AX_UINT8 = 1;
        public static readonly int AX_INT16 = 2;
        public static readonly int AX_UINT16 = 3;
        public static readonly int AX_INT32 = 4;
        public static readonly int AX_UINT32 = 5;
        public static readonly int AX_INT64 = 6;
        public static readonly int AX_UINT64 = 7;
        public static readonly int AX_FLOAT32 = 8;
        public static readonly int AX_FLOAT64 = 9;
        public static readonly int AX_STR_ACP = 10;
        public static readonly int AX_STR_UTF8 = 11;
        public static readonly int AX_STR_UTF16 = 12;
        
#if UNITY_IPHONE
        //Not support yet!
	
#elif UNITY_ANDROID
        
        static private AndroidJavaClass clsAxTrace = new AndroidJavaClass("com.thecodeway.axtrace.AxTrace");
        static public void SetTraceServer(string ip, int port)
        {
        		clsAxTrace.CallStatic("SetTraceServer", ip, port); 
        }
        static public void Trace(string format, params object[] args)
        {
            string final = System.String.Format(format, args);
            clsAxTrace.CallStatic("Trace", final, null); 
        }
        static public void TraceEx(int idwindow, int idStyle, string format, params object[] args)
        {
            string final = System.String.Format(format, args);
            clsAxTrace.CallStatic("TraceEx", idwindow, idStyle, final, null); 
        }
        
        static public void Value(string name, sbyte value)
        {
            clsAxTrace.CallStatic("_value", 0, 0, name, AX_INT8, BitConverter.GetBytes(value));
        }
        static public void Value(string name, byte value)
        {
            clsAxTrace.CallStatic("_value", 0, 0, name, AX_UINT8, BitConverter.GetBytes(value));
        }
        static public void Value(string name, short value)
        {
            clsAxTrace.CallStatic("_value", 0, 0, name, AX_INT16, BitConverter.GetBytes(value));
        }
        static public void Value(string name, ushort value)
        {
            clsAxTrace.CallStatic("_value", 0, 0, name, AX_UINT16, BitConverter.GetBytes(value));
        }
        static public void Value(string name, int value)
        {
            clsAxTrace.CallStatic("_value", 0, 0, name, AX_INT32, BitConverter.GetBytes(value));
        }
        static public void Value(string name, uint value)
        {
            clsAxTrace.CallStatic("_value", 0, 0, name, AX_UINT32, BitConverter.GetBytes(value));
        }
        static public void Value(string name, long value)
        {
            clsAxTrace.CallStatic("_value", 0, 0, name, AX_INT64, BitConverter.GetBytes(value));
        }
        static public void Value(string name, ulong value)
        {
            clsAxTrace.CallStatic("_value", 0, 0, name, AX_UINT64, BitConverter.GetBytes(value));
        }
        static public void Value(string name, float value)
        {
            clsAxTrace.CallStatic("_value", 0, 0, name, AX_FLOAT32, BitConverter.GetBytes(value));
        }
        static public void Value(string name, double value)
        {
            clsAxTrace.CallStatic("_value", 0, 0, name, AX_FLOAT64, BitConverter.GetBytes(value));
        }
        static public void Value(string name, string value)
        {
            clsAxTrace.CallStatic("Value", name, value);  	
        }

        static public void ValueEx(int idwindow, int idStyle, string name, sbyte value)
        {
            clsAxTrace.CallStatic("_value", idwindow, idStyle, name, AX_INT8, BitConverter.GetBytes(value));
        }
        static public void Value(int idwindow, int idStyle, string name, byte value)
        {
            clsAxTrace.CallStatic("_value", idwindow, idStyle, name, AX_UINT8, BitConverter.GetBytes(value));
        }
        static public void ValueEx(int idwindow, int idStyle, string name, short value)
        {
            clsAxTrace.CallStatic("_value", idwindow, idStyle, name, AX_INT16, BitConverter.GetBytes(value));
        }
        static public void ValueEx(int idwindow, int idStyle, string name, ushort value)
        {
            clsAxTrace.CallStatic("_value", idwindow, idStyle, name, AX_UINT16, BitConverter.GetBytes(value));
        }
        static public void ValueEx(int idwindow, int idStyle, string name, int value)
        {
            clsAxTrace.CallStatic("_value", idwindow, idStyle, name, AX_INT32, BitConverter.GetBytes(value));
        }
        static public void ValueEx(int idwindow, int idStyle, string name, uint value)
        {
            clsAxTrace.CallStatic("_value", idwindow, idStyle, name, AX_UINT32, BitConverter.GetBytes(value));
        }
        static public void ValueEx(int idwindow, int idStyle, string name, long value)
        {
            clsAxTrace.CallStatic("_value", idwindow, idStyle, name, AX_INT64, BitConverter.GetBytes(value));
        }
        static public void ValueEx(int idwindow, int idStyle, string name, ulong value)
        {
            clsAxTrace.CallStatic("_value", idwindow, idStyle, name, AX_UINT64, BitConverter.GetBytes(value));
        }
        static public void ValueEx(int idwindow, int idStyle, string name, float value)
        {
            clsAxTrace.CallStatic("_value", idwindow, idStyle, name, AX_FLOAT32, BitConverter.GetBytes(value));
        }
        static public void ValueEx(int idwindow, int idStyle, string name, double value)
        {
            clsAxTrace.CallStatic("_value", idwindow, idStyle, name, AX_FLOAT64, BitConverter.GetBytes(value));
        }
        static public void ValueEx(int idwindow, int idStyle, string name, string value)
        {
            clsAxTrace.CallStatic("Value", name, value);  	
        }
                
#else

        [DllImport("axtrace")]
        static extern void AxTrace_Init(byte[] traceServer, int tracePort);

        [DllImport("axtrace")]
        static extern void AxTrace_InsertLogA(int idWindow, int idStyle, int isUTF8, byte[] stringBuf);

        [DllImport("axtrace")]
        static extern void AxTrace_InsertLogW(int idWindow, int idStyle, short[] stringBuf);

        [DllImport("axtrace")]
        static extern void AxTrace_WatchValue(int idWindow, int idStyle, int valueType, byte[] valueName, int valueNameLength, byte[] value);

        static public void SetTraceServer(string ip, int port)
        {
            AxTrace_Init(System.Text.Encoding.UTF8.GetBytes(ip), port);
        }

        static public void Trace(string format, params object[] args)
        {
            string final = System.String.Format(format, args);
            AxTrace_InsertLogA(0, 0, 1, System.Text.Encoding.UTF8.GetBytes(final));
        }
        static public void TraceEx(int idwindow, int idStyle, string format, params object[] args)
        {
            string final = System.String.Format(format, args);
            AxTrace_InsertLogA(idwindow, idStyle, 1, System.Text.Encoding.UTF8.GetBytes(final));
        }

        static public void Value(string name, sbyte value)
        {
            AxTrace_WatchValue(0, 0, AX_INT8, System.Text.Encoding.UTF8.GetBytes(name), name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, byte value)
        {
            AxTrace_WatchValue(0, 0, AX_UINT8, System.Text.Encoding.UTF8.GetBytes(name), name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, short value)
        {
            AxTrace_WatchValue(0, 0, AX_INT16, System.Text.Encoding.UTF8.GetBytes(name), name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, ushort value)
        {
            AxTrace_WatchValue(0, 0, AX_UINT16, System.Text.Encoding.UTF8.GetBytes(name), name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, int value)
        {
            AxTrace_WatchValue(0, 0, AX_INT32, System.Text.Encoding.UTF8.GetBytes(name), name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, uint value)
        {
            AxTrace_WatchValue(0, 0, AX_UINT32, System.Text.Encoding.UTF8.GetBytes(name), name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, long value)
        {
            AxTrace_WatchValue(0, 0, AX_INT64, System.Text.Encoding.UTF8.GetBytes(name), name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, ulong value)
        {
            AxTrace_WatchValue(0, 0, AX_UINT64, System.Text.Encoding.UTF8.GetBytes(name), name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, float value)
        {
            AxTrace_WatchValue(0, 0, AX_FLOAT32, System.Text.Encoding.UTF8.GetBytes(name), name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, double value)
        {
            AxTrace_WatchValue(0, 0, AX_FLOAT64, System.Text.Encoding.UTF8.GetBytes(name), name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, string value)
        {
            AxTrace_WatchValue(0, 0, AX_STR_UTF8, System.Text.Encoding.UTF8.GetBytes(name), name.Length, System.Text.Encoding.UTF8.GetBytes(value));
        }
#endif
    }

}
