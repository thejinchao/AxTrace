using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;

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
        
        [DllImport("axtrace")]
        static extern void AxTrace_Init();

        [DllImport("axtrace")]
        static extern void AxTrace_InsertLogA(int idWindow, int idStyle, int isUTF8, byte[] stringBuf);

        [DllImport("axtrace")]
        static extern void AxTrace_InsertLogW(int idWindow, int idStyle, short[] stringBuf);

        [DllImport("axtrace")]
        static extern void AxTrace_WatchValue(int idWindow, int idStyle, int valueType, byte[] valueName, int valueNameLength, byte[] value);

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

        static private byte[] _getStringWithBlank(string value)
        {
            byte[] _value = System.Text.Encoding.UTF8.GetBytes(value);
            byte[] _value_with_blank = new byte[_value.Length + 1];
            _value.CopyTo(_value_with_blank, 0);
            _value_with_blank[_value.Length] = 0;
            return _value_with_blank;
        }

        static public void Value(string name, sbyte value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_INT8, _name, _name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, byte value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_UINT8, _name, _name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, short value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_INT16, _name, _name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, ushort value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_UINT16, _name, _name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, int value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_INT32, _name, _name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, uint value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_UINT32, _name, _name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, long value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_INT64, _name, _name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, ulong value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_UINT64, _name, _name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, float value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_FLOAT32, _name, _name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, double value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_FLOAT64, _name, _name.Length, BitConverter.GetBytes(value));
        }
        static public void Value(string name, string value)
        {
            byte[] _name = _getStringWithBlank(name);
            AxTrace_WatchValue(0, 0, AX_STR_UTF8, _name, _name.Length, _getStringWithBlank(value));
        }
    }

}
