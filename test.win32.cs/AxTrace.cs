using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;

static class Axia
{
    [DllImport("Kernel32.dll")]
    static extern IntPtr LoadLibrary(string lpFileName);

    [DllImport("Kernel32.dll")]
    static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

    delegate int AxTrace_Init();
    delegate int AxTrace_InsertLogA(int idWindow, int idStyle, int isUTF8, string stringBuf);
    delegate int AxTrace_InsertLogW(int idWindow, int idStyle, short[] stringBuf);
    delegate int AxTrace_WatchValue(int idWindow, int idStyle, int valueType, string valueName, int valueNameLength, byte[] value);

    class GLOBAL_DATA
    {
        public AxTrace_Init funcInit;
        public AxTrace_InsertLogA funcInsertLogA;
        public AxTrace_InsertLogW funcInsertLogW;
        public AxTrace_WatchValue funcWatchValue;
        public bool init;
        public bool initSuccess;

        public GLOBAL_DATA()
        {
            funcInit = null;
            funcInsertLogA = null;
            funcInsertLogW = null;
            funcWatchValue = null;
            init = false;
            initSuccess = false;
        }
    };

    static private object m_lockGlobalData = new object();
    static private GLOBAL_DATA m_theGlobalData = new GLOBAL_DATA();

    static private bool _LoadGlobalData()
    {
        if (m_theGlobalData.init) return m_theGlobalData.initSuccess;

        lock (m_lockGlobalData)
        {
            m_theGlobalData.init = true;
            m_theGlobalData.initSuccess = false;

            IntPtr hModule = LoadLibrary("axtrace.dll");
            if (hModule == IntPtr.Zero) return false;

            IntPtr addrInit = GetProcAddress(hModule, "AxTrace_Init");
            IntPtr addrInsertLogA = GetProcAddress(hModule, "AxTrace_InsertLogA");
            IntPtr addrInsertLogW = GetProcAddress(hModule, "AxTrace_InsertLogW");
            IntPtr addrWatchValue = GetProcAddress(hModule, "AxTrace_WatchValue");
            if (addrInit == IntPtr.Zero || addrInsertLogA == IntPtr.Zero || addrInsertLogW == IntPtr.Zero || addrWatchValue == IntPtr.Zero) return false;

            m_theGlobalData.funcInit = (AxTrace_Init)Marshal.GetDelegateForFunctionPointer(addrInit, typeof(AxTrace_Init));
            m_theGlobalData.funcInsertLogA = (AxTrace_InsertLogA)Marshal.GetDelegateForFunctionPointer(addrInsertLogA, typeof(AxTrace_InsertLogA));
            m_theGlobalData.funcInsertLogW = (AxTrace_InsertLogW)Marshal.GetDelegateForFunctionPointer(addrInsertLogA, typeof(AxTrace_InsertLogW));
            m_theGlobalData.funcWatchValue = (AxTrace_WatchValue)Marshal.GetDelegateForFunctionPointer(addrWatchValue, typeof(AxTrace_WatchValue));
            m_theGlobalData.initSuccess = true;
        }

        return m_theGlobalData.initSuccess;
    }

    static public void AxTrace(int idwindow, int idStyle, string value)
    {
        if (!_LoadGlobalData()) return;
        m_theGlobalData.funcInsertLogA(idwindow, idStyle, 0, value);
    }
    static public void AxTrace_UTF8(int idwindow, int idStyle, string value)
    {
        if (!_LoadGlobalData()) return;
        m_theGlobalData.funcInsertLogA(idwindow, idStyle, 1, value);
    }
}

