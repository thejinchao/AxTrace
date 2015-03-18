package com.thecodeway.axtrace;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.concurrent.atomic.AtomicBoolean;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.util.Log;

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
    
    private static String mServerAddress="127.0.0.1";
    private static int mServerPort = 1978;
    
	static public void Trace(int style, String param, Object... args) {
		String finalString = String.format(param, args);
		
		_trace(style, finalString);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////
	//private functions
	////////////////////////////////////////////////////////////////////////////////////////////

	static private class InnerLogData{
		int style;
		int pid;
		int tid;
		String data;
		AtomicBoolean done;
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
	}
	
	static private class ThreadValue {
		public boolean mInitSuccessed = false;
		public NetworkContex mNetwork = null;
	}
	
	private static class BackgroundThread implements Runnable {
		public Handler mHandler;
		public Object mInitLock = new Object();
		public boolean mInitSuccessed=false;
		private NetworkContex mNetwork;
		
		@Override
		public void run() {
			try {
			Looper.prepare();
			
			//connect to server
			mNetwork = new NetworkContex();
			if(!(mNetwork.connect(mServerAddress, mServerPort)))
				return;
			mInitSuccessed = true;
			
			//message handle
			mHandler = new Handler(){
				@Override
				public void handleMessage(Message msg){
					switch(msg.what) {
					case AXTRACE_CMD_TYPE_TRACE:
						InnerLogData logData = (InnerLogData)msg.obj;
						mNetwork.sendLog(logData);
						
					    synchronized(logData.done) {
					    	logData.done.set(true);
					    	logData.done.notify();
					    }
						break;
						
					case AXTRACE_CMD_TYPE_VALUE:
						//TODO: ...
						break;
					}
				}			
			};
			}finally {
				//notify 
			    synchronized(mInitLock) {
			    	mInitLock.notify();
			    }
			}
			
			Looper.loop();
		}
	}
	
	static BackgroundThread mBackgroundThread = null;
	
	//connect to server
	static private boolean _initBackgroundThread() {
		if(mBackgroundThread!=null) {
			//already inited
			return mBackgroundThread.mInitSuccessed; 
		}
		
		//create background thread
		mBackgroundThread = new BackgroundThread();
		new Thread(mBackgroundThread).start();
		
		//wait init completed
		synchronized(mBackgroundThread.mInitLock) {
			try {
				mBackgroundThread.mInitLock.wait();
			}catch(InterruptedException e) {
			}
		}
		
		return mBackgroundThread.mInitSuccessed;
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
		logData.pid = (int)Process.myPid();
		logData.tid = (int)Process.myTid();
		logData.data = finalString;
		logData.done = new AtomicBoolean();
		logData.done.set(false);
		
		//is in main thread?
		if(Thread.currentThread() == Looper.getMainLooper().getThread()) {
	
			//init a backgournd thread to send message(android can't use socket api in main thread)
			if(!_initBackgroundThread()) return;
			
			//send to backgournd thread
			Message msg = mBackgroundThread.mHandler.obtainMessage(
					AXTRACE_CMD_TYPE_TRACE, logData);
			msg.sendToTarget(); 
			
			if(logData.done.get()) return;
			
		    synchronized(logData.done) {
	    		try {
	    			logData.done.wait();
	    		}catch(Exception ex) {
	    		}
		    }	
		} else {
			
			//init current thread, connect to server
			if(!_initCurrentThread()) {
				return;
			}
			
			//send in current thread
			mThreadData.get().mNetwork.sendLog(logData);
		}
		
	}

}
