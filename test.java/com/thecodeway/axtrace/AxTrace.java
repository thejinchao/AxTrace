package com.thecodeway.axtrace;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.lang.management.ManagementFactory;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class AxTrace {
	public static final int AXT_TRACE = 1;
    public static final int AXT_DEBUG = 1;
    public static final int AXT_INFO = 2;
    public static final int AXT_WARN = 3;
    public static final int AXT_ERROR = 4;
    public static final int AXT_FATAL = 5;
    public static final int AXT_USERDEF = 10;
	
    public static final byte AXTRACE_CMD_TYPE_TRACE = 1;
    public static final byte AXTRACE_CMD_TYPE_VALUE = 2;
    
	static public final int AXV_INT8 = 0;
	static public final int AXV_UINT8 = 1;
	static public final int AXV_INT16 = 2;
	static public final int AXV_UINT16 = 3;
	static public final int AXV_INT32 = 4;
	static public final int AXV_UINT32 = 5;
	static public final int AXV_INT64 = 6;
	static public final int AXV_UINT64 = 7;
	static public final int AXV_FLOAT32 = 8;
	static public final int AXV_FLOAT64 = 9;
	static public final int AXV_STR_ACP = 10;
	static public final int AXV_STR_UTF8 = 11;
	static public final int AXV_STR_UTF16 = 12;
	
    //private static final short AX_CODEPAGE_ACP = 0;
    private static final short AX_CODEPAGE_UTF8 = 1;
    //private static final short AX_CODEPAGE_UTF16 = 2;
    
    private static final int AXTRACE_COMMON_HEAD_SIZE = 16;
    private static final int AXTRACE_TRACE_HEAD_SIZE = 4; 
    private static final int AXTRACE_VALUE_HEAD_SIZE = 8;
    
    private static String mServerAddress="127.0.0.1";
    private static int mServerPort = 1978;
    
    static public synchronized void SetTraceServer(String ip, int port) {
		mServerAddress = ip;
		mServerPort = port;
	}
    
	static public void Trace(int style, String param, Object... args) {
		String finalString = String.format(param, args);
		
		_trace(style, finalString);
	}
	
	static public void Value(int style, String name, byte value) {
		byte[] v = new byte[1];
		v[0] = value;
		_value(style, name, AXV_INT8, v);
	}
	
	static public void Value(int style, String name, int value) {
		byte[] v = new byte[4];
		for(int i=0; i<4; i++)
			v[i] = (byte)(value>>(i*8));
		_value(style, name, AXV_INT32, v);
	}
	
	static public void Value(int style, String name, long value) {
		byte[] v = new byte[8];
		for(int i=0; i<8; i++)
			v[i] = (byte)(value>>(i*8));
		_value(style, name, AXV_INT64, v);
	}
	
	static public void Value(int style, String name, float value) {
		byte[] v = new byte[4];
		int _v = Float.floatToIntBits(value);
		for(int i=0; i<4; i++)
			v[i] = (byte)(_v>>(i*8));
		_value(style, name, AXV_FLOAT32, v);
	}
	
	static public void Value(int style, String name, double value) {
		byte[] v = new byte[8];
		long _v = Double.doubleToLongBits(value);
		for(int i=0; i<8; i++)
			v[i] = (byte)(_v>>(i*8));
		_value(style, name, AXV_FLOAT64, v);
	}
	
	static public void Value(int style, String name, String value) {
		try {
			byte[] v = value.getBytes("UTF-8");
			int length = v.length;
			
			byte[] _v = new byte[length+1];
			System.arraycopy(v, 0, _v, 0, length);
			_v[length]=0;
			
			_value(style, name, AXV_STR_UTF8, _v);
		}catch(UnsupportedEncodingException e) {
			
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////
	//private functions
	////////////////////////////////////////////////////////////////////////////////////////////

	static private class InnerLogData{
		int style;
		int pid;
		int tid;
		String data;
	}
	static private class InnerValueData{
		int style;
		int pid;
		int tid;
		String name;
		int valueType;
		byte[] value;
	}
		
	private static long getCurrentProcessId() {
	    // Note: may fail in some JVM implementations
	    // therefore fallback has to be provided

	    // something like '<pid>@<hostname>', at least in SUN / Oracle JVMs
	    final String jvmName = ManagementFactory.getRuntimeMXBean().getName();
	    final int index = jvmName.indexOf('@');

	    if (index < 1) {
	        // part before '@' empty (index = 0) / '@' not found (index = -1)
	        return 0;
	    }

	    try {
	        return Long.parseLong(jvmName.substring(0, index));
	    } catch (NumberFormatException e) {
	        // ignore
	    }
	    return 0;
	}
	
	private static class NetworkContex {
		private Socket mSocket=null;
		public boolean connect(String server_ip, int server_port) {
			try {
				mSocket = new Socket(server_ip, server_port);
			}catch(Exception ex) {
				return false;
			}
			return true;
		}
		
		public void sendLog(InnerLogData logData)  {
			try {
				byte[] byteString = logData.data.getBytes("UTF-8");
				int finalLength = AXTRACE_COMMON_HEAD_SIZE+AXTRACE_TRACE_HEAD_SIZE+byteString.length+1;
				
				ByteBuffer buf = ByteBuffer.wrap(
						new byte[finalLength]).order(ByteOrder.LITTLE_ENDIAN);
				
				buf.putShort((short)finalLength);
				buf.put((byte)'A');
				buf.put((byte)AXTRACE_CMD_TYPE_TRACE);
				buf.putInt(logData.pid);
				buf.putInt(logData.tid);
				buf.putInt(logData.style);
				
				buf.putShort((short)AX_CODEPAGE_UTF8);
				buf.putShort((short)(byteString.length+1));
				buf.put(byteString);
				buf.put((byte)0);
				
				mSocket.getOutputStream().write(buf.array());

			}catch(UnsupportedEncodingException e) {
				
			}catch(IOException ex) {
				
			}
		}
		
		public void sendValue(InnerValueData valueData)  {
			try {
				int finalLength = AXTRACE_COMMON_HEAD_SIZE+AXTRACE_VALUE_HEAD_SIZE+
						valueData.name.length()+1+valueData.value.length;
				
				ByteBuffer buf = ByteBuffer.wrap(
						new byte[finalLength]).order(ByteOrder.LITTLE_ENDIAN);
				buf.putShort((short)finalLength);
				buf.put((byte)'A');
				buf.put((byte)AXTRACE_CMD_TYPE_VALUE);
				buf.putInt(valueData.pid);
				buf.putInt(valueData.tid);
				buf.putInt(valueData.style);
				
				buf.putInt(valueData.valueType);
				buf.putShort((short)(valueData.name.length()+1));
				buf.putShort((short)(valueData.value.length));
				buf.put(valueData.name.getBytes("UTF-8"));
				buf.put((byte)0);
				buf.put(valueData.value);
				
				mSocket.getOutputStream().write(buf.array());
				
			}catch(UnsupportedEncodingException e) {
				
			}catch(IOException ex) {
				
			}
		}		
	}
	
	static private class ThreadValue {
		public boolean mInitSuccessed = false;
		public NetworkContex mNetwork = null;
	}
	
	private static ThreadLocal<ThreadValue> mThreadData = new ThreadLocal<ThreadValue>();
	
	static private boolean _initCurrentThread() {
		if(mThreadData.get()!=null) {
			boolean inited = mThreadData.get().mInitSuccessed;
			return inited;
		}
		
		//create current thread local data
		ThreadValue data = new ThreadValue();
		data.mNetwork = new NetworkContex();
		data.mInitSuccessed = data.mNetwork.connect(mServerAddress, mServerPort);
		
		mThreadData.set(data);
		
		return data.mInitSuccessed;
	}
	
	static private void _trace(int style, String finalString){
		InnerLogData logData = new InnerLogData();
		logData.style = style;
		logData.pid = (int)getCurrentProcessId();
		logData.tid = (int)Thread.currentThread().getId();
		logData.data = finalString;
		
		//init current thread, connect to server
		if(!_initCurrentThread()) {
			return;
		}
			
		//send in current thread
		mThreadData.get().mNetwork.sendLog(logData);
	}
	
	static public void _value(int style, String name, int valueType, byte[] value)
	{
		InnerValueData valueData = new InnerValueData();
		valueData.style = style;
		valueData.pid = (int)getCurrentProcessId();
		valueData.tid = (int)Thread.currentThread().getId();
		valueData.name = name;
		valueData.valueType = valueType;
		valueData.value = value;
		
		//init current thread, connect to server
		if(!_initCurrentThread()) {
			return;
		}
			
		//send in current thread
		mThreadData.get().mNetwork.sendValue(valueData);
	}

}
