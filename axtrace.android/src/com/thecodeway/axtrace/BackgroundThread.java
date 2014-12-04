package com.thecodeway.axtrace;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.zeromq.ZMQ;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

public class BackgroundThread implements Runnable {

	static public final int AXTRACE_MAGIC_HEAD = 0x5841;
	static public final int AXTRACE_HEAD_SIZE = 16;
	
	static public final int ATT_LOG = 1;
	static public final int ATT_VALUE = 2;
	
	static public final int ATC_ACP = 0;
	static public final int ATC_UTF8 = 1;
	static public final int ATC_UTF16 = 2;
	
	static public final int AX_INT8=0;
	static public final int AX_UINT8=1;
	static public final int AX_INT16=2;
	static public final int AX_UINT16=3;
	static public final int AX_INT32=4;
	static public final int AX_UINT32=5;
	static public final int AX_INT64=6;
	static public final int AX_UINT64=7;
	static public final int AX_FLOAT32=8;
	static public final int AX_FLOAT64=9;
	static public final int AX_STR_ACP=10;
	static public final int AX_STR_UTF8=11;
	static public final int AX_STR_UTF16=12;
	
	static public String TraceServer="127.0.0.1";
	static public int TraceServerPort=1978;
	
	static public class InnerLogData{
		int pid;
		int tid;
		int idWindow;
		int idStyle;
		String strTrace;
	}
	
	static public class InnerValueData{
		int pid;
		int tid;
		int idWindow;
		int idStyle;
		String strName;
		int valueType;
		byte[] value;
	}
	
	public Handler mHandler;
	public Object mInitLock = new Object();
	public boolean mInitSuccessed=false;
	
	private ZMQ.Context contextZMQ;
	private ZMQ.Socket socketZMQ;
	
	@Override
	public void run() {
		//init zeromq 
		contextZMQ = ZMQ.context(1);
	    socketZMQ = contextZMQ.socket(ZMQ.PUSH);
	    socketZMQ.connect("tcp://" + TraceServer + ":" + TraceServerPort);
	    
		Looper.prepare();
	
		mHandler = new Handler(){
			@Override
			public void handleMessage(Message msg){
				switch(msg.what) {
				case ATT_LOG:
					InnerLogData logData = (InnerLogData)msg.obj;
					_InsertLog(logData);
					break;
					
				case ATT_VALUE:
					InnerValueData valueData = (InnerValueData)msg.obj;
					_InsertValue(valueData);
					break;
				}
			}			
		};
		
		//notify 
	    synchronized(mInitLock) {
	    	mInitSuccessed = true;
	    	mInitLock.notify();
	    }
	    
		Looper.loop();
	}
	
	private void _InsertLog(InnerLogData logData)
	{
		try {
			byte[] byteString = logData.strTrace.getBytes("UTF-8");
		
			int finalLength = 4*2 +byteString.length+1;
			
			ByteBuffer buf = ByteBuffer.wrap(new byte[AXTRACE_HEAD_SIZE+finalLength]).order(ByteOrder.LITTLE_ENDIAN);
			buf.putShort((short)AXTRACE_MAGIC_HEAD);
			buf.putShort((short)finalLength);
			buf.putInt(logData.pid);
			buf.putInt(logData.tid);
			buf.putShort((short)ATT_LOG);
			buf.put((byte)logData.idWindow);
			buf.put((byte)logData.idStyle);
			//-----
			buf.putInt(ATC_UTF8);
			buf.putInt(byteString.length);
			buf.put(byteString);	
			//-----
			
	        socketZMQ.send(buf.array(), 0, buf.position(), 0); 
        
		}catch(UnsupportedEncodingException e) {
			
		}
	}

	private void _InsertValue(InnerValueData logData) {
		try {
			byte[] byteName = logData.strName.getBytes("UTF-8");	
			
			int finalLength = 4*3 + byteName.length + 1 + logData.value.length;
			
			ByteBuffer buf = ByteBuffer.wrap(new byte[AXTRACE_HEAD_SIZE+finalLength]).order(ByteOrder.LITTLE_ENDIAN);
			buf.putShort((short)AXTRACE_MAGIC_HEAD);
			buf.putShort((short)finalLength);
			buf.putInt(logData.pid);
			buf.putInt(logData.tid);
			buf.putShort((short)ATT_VALUE);
			buf.put((byte)logData.idWindow);
			buf.put((byte)logData.idStyle);
			//-----
			buf.putInt(logData.valueType);
			buf.putInt(byteName.length);
			buf.put(byteName);
			buf.putInt(logData.value.length);
			buf.put(logData.value);
			//-----
			
	        socketZMQ.send(buf.array(), 0, buf.position(), 0); 
		}catch(UnsupportedEncodingException e) {
			
		}
	}
}
