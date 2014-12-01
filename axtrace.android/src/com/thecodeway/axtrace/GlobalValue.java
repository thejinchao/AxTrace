package com.thecodeway.axtrace;

public class GlobalValue {
	//is inited
	public boolean isInited = false;
	// if already init, is successed?
	public boolean isInitSucc = false;		
	// background thread to connect axtrace server
	public BackgroundThread background=null;
	
	//------------------
	//get instance
	//------------------
	private static class GlobalValueInstance {
		private static final GlobalValue sInstance=new GlobalValue();
	}
	
	//thread safe(from Effective Java)
	public static GlobalValue getInstance() {
		return GlobalValueInstance.sInstance;
	}
	
	//private construct 
	private GlobalValue() {
		isInited = false;
		isInitSucc = false;
		background = new BackgroundThread();
	}
}
