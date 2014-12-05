package com.thecodeway.axtrace;

import java.io.UnsupportedEncodingException;

import android.os.Message;
import android.os.Process;

public class AxTrace {
	static public final int MaxWindowsID = 255;
	static public final int MaxStyleID = 255;
	static public final int MaxLogLengthInByte = 65534;
	static public final int MaxValueLengthInByte = 65534;
	static public final int MaxValueNameLength = 128;
	
	static public synchronized void SetTraceServer(String ip, int port) {
		BackgroundThread.TraceServer = ip;
		BackgroundThread.TraceServerPort = port;
		
		_Init();
	}
	
	static public void Trace(String param, Object... args) {
		String finalString = String.format(param, args);
		_trace(0, 0, finalString);
	}
	
	static public void TraceEx(int idWindow, int idStyle, String param, Object... args) {
		String finalString = String.format(param, args);
		_trace(idWindow, idStyle, finalString);
	}
	
	static public void Value(String name, byte value) {
		byte[] v = new byte[1];
		v[0] = value;
		_value(0, 0, name, BackgroundThread.AX_INT8, v);
	}

	static public void Value(String name, short value) {
		byte[] v = new byte[2];
		v[0] = (byte)(value);
		v[1] = (byte)(value>>8);
		_value(0, 0, name, BackgroundThread.AX_INT16, v);
	}

	static public void Value(String name, int value) {
		byte[] v = new byte[4];
		for(int i=0; i<4; i++)
			v[i] = (byte)(value>>(i*8));
		_value(0, 0, name, BackgroundThread.AX_INT32, v);
	}
	
	static public void Value(String name, long value) {
		byte[] v = new byte[8];
		for(int i=0; i<8; i++)
			v[i] = (byte)(value>>(i*8));
		_value(0, 0, name, BackgroundThread.AX_INT64, v);
	}
	
	static public void Value(String name, float value) {
		byte[] v = new byte[4];
		int _v = Float.floatToIntBits(value);
		for(int i=0; i<4; i++)
			v[i] = (byte)(_v>>(i*8));
		_value(0, 0, name, BackgroundThread.AX_FLOAT32, v);
	}
	
	static public void Value(String name, double value) {
		byte[] v = new byte[8];
		long _v = Double.doubleToLongBits(value);
		for(int i=0; i<8; i++)
			v[i] = (byte)(_v>>(i*8));
		_value(0, 0, name, BackgroundThread.AX_FLOAT64, v);
	}
	
	static public void Value(String name, String value) {
		try {
			byte[] v = value.getBytes("UTF-8");
			int length = v.length;
			
			byte[] _v = new byte[length+1];
			System.arraycopy(v, 0, _v, 0, length);
			_v[length]=0;
			
			_value(0, 0, name, BackgroundThread.AX_STR_UTF8, _v);
		}catch(UnsupportedEncodingException e) {
			
		}
	}
	
	static public void ValueEx(int idWindow, int idStyle, String name, byte value) {
		byte[] v = new byte[1];
		v[0] = value;
		_value(idWindow, idStyle, name, BackgroundThread.AX_INT8, v);
	}
	
	static public void ValueEx(int idWindow, int idStyle, String name, short value) {
		byte[] v = new byte[2];
		v[0] = (byte)(value);
		v[1] = (byte)(value>>8);
		_value(idWindow, idStyle, name, BackgroundThread.AX_INT16, v);
	}
		
	static public void ValueEx(int idWindow, int idStyle, String name, int value) {
		byte[] v = new byte[4];
		for(int i=0; i<4; i++)
			v[i] = (byte)(value>>(i*8));
		_value(idWindow, idStyle, name, BackgroundThread.AX_INT32, v);
	}
	
	static public void ValueEx(int idWindow, int idStyle, String name, long value) {
		byte[] v = new byte[8];
		for(int i=0; i<8; i++)
			v[i] = (byte)(value>>(i*8));
		_value(idWindow, idStyle, name, BackgroundThread.AX_INT64, v);
	}
	
	static public void ValueEx(int idWindow, int idStyle, String name, float value) {
		byte[] v = new byte[4];
		int _v = Float.floatToIntBits(value);
		for(int i=0; i<4; i++)
			v[i] = (byte)(_v>>(i*8));
		_value(idWindow, idStyle, name, BackgroundThread.AX_FLOAT32, v);
	}
	
	static public void ValueEx(int idWindow, int idStyle, String name, double value) {
		byte[] v = new byte[8];
		long _v = Double.doubleToLongBits(value);
		for(int i=0; i<8; i++)
			v[i] = (byte)(_v>>(i*8));
		_value(idWindow, idStyle, name, BackgroundThread.AX_FLOAT64, v);
	}
	
	static public void ValueEx(int idWindow, int idStyle, String name, String value) {
		try {
			byte[] v = value.getBytes("UTF-8");
			int length = v.length;
			
			byte[] _v = new byte[length+1];
			System.arraycopy(v, 0, _v, 0, length);
			_v[length]=0;
			
			_value(idWindow, idStyle, name, BackgroundThread.AX_STR_UTF8, _v);
		}catch(UnsupportedEncodingException e) {
			
		}
	}
	
	static public void _trace(int idWindow, int idStyle, String strTrace) {
		if(!_Init()) return;
		
		//check range
		if(idWindow<0 || idWindow>MaxWindowsID) return;
		if(idStyle<0 || idStyle>MaxStyleID) return;
		if(strTrace.length()>MaxLogLengthInByte) return;
		
		GlobalValue _global = GlobalValue.getInstance();

		BackgroundThread.InnerLogData logData = new BackgroundThread.InnerLogData();
		logData.pid = (int)Process.myPid();
		logData.tid = (int)Thread.currentThread().getId();
		logData.idWindow = idWindow;
		logData.idStyle = idStyle;
		logData.strTrace = strTrace;
		
		Message msg = _global.background.mHandler.obtainMessage(
				BackgroundThread.ATT_LOG, logData);
		msg.sendToTarget();
	}
	
	static public void _value(int idWindow, int idStyle, String name, int valueType, byte[] value) {
		if(!_Init()) return;
		
		//check range
		if(idWindow<0 || idWindow>MaxWindowsID) return;
		if(idStyle<0 || idStyle>MaxStyleID) return;
		if(valueType<BackgroundThread.AX_INT8 || valueType>BackgroundThread.AX_STR_UTF16) return;
		if(name.length()>MaxValueNameLength) return;
		if(value.length>MaxValueLengthInByte) return;
		
		GlobalValue _global = GlobalValue.getInstance();
		
		BackgroundThread.InnerValueData valueData = new BackgroundThread.InnerValueData();
		valueData.pid = (int)Process.myPid();
		valueData.tid = (int)Thread.currentThread().getId();
		valueData.idWindow = idWindow;
		valueData.idStyle = idStyle;
		valueData.strName = name;
		valueData.valueType = valueType;
		valueData.value = value;

		Message msg = _global.background.mHandler.obtainMessage(
				BackgroundThread.ATT_VALUE, valueData);
		msg.sendToTarget();
	}
	
	static private boolean _Init() {
		GlobalValue _global = GlobalValue.getInstance();
		if(_global.isInited) return _global.isInitSucc;
		
		//init(thread safe)
		_Init_Gloabl_Value(_global);
		
		return _global.isInitSucc;
	}
	
	static private synchronized void _Init_Gloabl_Value(GlobalValue _global) {
		if(_global.isInited) {
			return; //already inited by other thread?
		}

		new Thread(_global.background).start();
		
		//wait init completed
		synchronized(_global.background.mInitLock) {
			while(!_global.background.mInitSuccessed) {
				try {
					_global.background.mInitLock.wait();
				}catch(InterruptedException e) {
					//finish init(failed)
					_global.isInited = true;
					_global.isInitSucc = false;
					return;
				}
			}
		}

		//finish init
		_global.isInited = true;
		_global.isInitSucc = true;
	}
}
