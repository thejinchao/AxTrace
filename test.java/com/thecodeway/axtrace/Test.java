package com.thecodeway.axtrace;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


public class Test
{
	private static ExecutorService threadPool = Executors.newFixedThreadPool(5);
	
	public static void main(String[] args){
		//test AxTrace
    	AxTrace.Log	(AxTrace.AXT_TRACE, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
    	AxTrace.Log(AxTrace.AXT_TRACE, "中文字符+Ascii");
    	AxTrace.Log(AxTrace.AXT_TRACE, "MultiLineTest\nLine1:第一行\nLine2:第二行\nLine%d:第三行",3);
    	
		//--------------------------
		//pressure test
		{
			int blank_Count=0;
			int step=1;
			int MAX_BLANK_COUNT=50;

			AxTrace.Log(AxTrace.AXT_TRACE, "<BEGIN>");
			for(int i=0; i<500; i++)
			{
				byte[] szTemp = new byte[blank_Count+1];

				int j=0;
				for(j=0; j<blank_Count; j++) szTemp[j]=' ';

				blank_Count+=step;
				if(blank_Count>=MAX_BLANK_COUNT) step=-1;
				if(blank_Count<=0)step=1;

				szTemp[j++] = '*';

				String str = new String(szTemp);
				AxTrace.Log(AxTrace.AXT_TRACE, str);
			}
			AxTrace.Log(AxTrace.AXT_TRACE, "<END>");
		}

		//--------------------------------------
		//thread pool press test
		{
			class TraceThread extends Thread {
				public TraceThread(String str) {
					strToShow = str;
				}
				private String strToShow;
				public void run() {
					AxTrace.Log(AxTrace.AXT_TRACE, strToShow);
                }
			}
			
			int blank_Count=0;
			int step=1;
			int MAX_BLANK_COUNT=50;
			AxTrace.Log(AxTrace.AXT_TRACE, "<MULTI THREAD BEGIN>");
			for(int i=0 ; i< 100 ; i++) {
				
				byte[] szTemp = new byte[blank_Count+1];
				
				int j;
				for(j=0; j<blank_Count; j++) szTemp[j]=' ';

				blank_Count+=step;
				if(blank_Count>=MAX_BLANK_COUNT) step=-1;
				if(blank_Count<=0)step=1;

				szTemp[j++] = '#';
				String r = String.format("[%03d]", i) + new String(szTemp);
				threadPool.submit(new TraceThread(r));
	        }	
			
			AxTrace.Log(AxTrace.AXT_TRACE, "<MULTI THREAD END>");
		}
		
    	
    	
		//--------------------------------------
		//value test
		{
			//test value
			AxTrace.Value(AxTrace.AXT_TRACE, "Int_Test", (int)-12345);
	
			AxTrace.Value(AxTrace.AXT_TRACE, "BYTE_MAX_VALUE", (byte)Byte.MAX_VALUE);
			AxTrace.Value(AxTrace.AXT_TRACE, "BYTE_MIN_VALUE", (byte)Byte.MIN_VALUE);
	
			AxTrace.Value(AxTrace.AXT_TRACE, "SHORT_MAX_VALUE", (short)Short.MAX_VALUE);
			AxTrace.Value(AxTrace.AXT_TRACE, "SHORT_MIN_VALUE", (short)Short.MIN_VALUE);
	
			AxTrace.Value(AxTrace.AXT_TRACE, "INT_MAX_VALUE", (int)Integer.MAX_VALUE);
			AxTrace.Value(AxTrace.AXT_TRACE, "INT_MIN_VALUE", (int)Integer.MIN_VALUE);
	
			AxTrace.Value(AxTrace.AXT_TRACE, "LONG_MAX_VALUE", (long)Long.MAX_VALUE);
			AxTrace.Value(AxTrace.AXT_TRACE, "LONG_MIN_VALUE", (long)Long.MIN_VALUE);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "Float_Test", (float)-3.1415926f);
			AxTrace.Value(AxTrace.AXT_TRACE, "Double_Test", (double)-3.141592653589793238462643383279);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "FLOAT_MAX_VALUE", (float)Float.MAX_VALUE);
			AxTrace.Value(AxTrace.AXT_TRACE, "FLOAT_MIN_VALUE", (float)Float.MIN_VALUE);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "DOUBLE_MAX_VALUE", (double)Double.MAX_VALUE);
			AxTrace.Value(AxTrace.AXT_TRACE, "DOUBLE_MIN_VALUE", (double)Double.MIN_VALUE);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "String_Test", "String 汉字");
		}
		
    			
		{

			//AxValue Pressure Test
			int start_blank=0;
			int start_step=1;
			int MAX_BLANK_COUNT=50;
	
			for(int i=0; i<100; i++)
			{
				AxTrace.Value(AxTrace.AXT_TRACE, "start_blank", start_blank);
				
				int blank_Count=start_blank;
				int step=start_step;
	
				for(int j=0; j<50; j++)
				{
					byte[] szTemp = new byte[blank_Count+1];
	
					int k=0;
					for(k=0; k<blank_Count; k++) szTemp[k]=' ';
	
					blank_Count+=step;
					if(blank_Count>=MAX_BLANK_COUNT) step=-1;
					if(blank_Count<=0)step=1;
	
					szTemp[k++] = '*';
	
					String value_name = "Value_"+ j;
					AxTrace.Value(AxTrace.AXT_TRACE, value_name, new String(szTemp));
				}
	
				start_blank += start_step;
				if(start_blank>=MAX_BLANK_COUNT) start_step=-1; 
				if(start_blank<=0) start_step=1;
			}
		}
		
		
		{
			class AxValueThread extends Thread {
				public AxValueThread(String name, String value) {
					strName = name; strValue = value;
				}
				private String strName;
				private String strValue;
				public void run() {
					AxTrace.Value(AxTrace.AXT_TRACE, strName, strValue);
                }
			}
			
			//AxValue Multi Thread Pressure Test
			int start_blank=0;
			int start_step=1;
			int MAX_BLANK_COUNT=50;
			
			for(int i=0; i<100; i++)
			{
				AxTrace.Value(AxTrace.AXT_TRACE, "start_blank_multiThread", start_blank);
				int blank_Count=start_blank;
				int step=start_step;

				for(int j=0; j<50; j++)
				{
					byte[] szTemp = new byte[blank_Count+1];

					int k=0;
					for(k=0; k<blank_Count; k++) szTemp[k]=' ';

					blank_Count+=step;
					if(blank_Count>=MAX_BLANK_COUNT) step=-1;
					if(blank_Count<=0)step=1;

					szTemp[k++] = '#';

					String value_name = "Value_" + j;

					Thread tt = new AxValueThread(value_name, new String(szTemp));
		            threadPool.submit(tt);
				}

				start_blank += start_step;
				if(start_blank>=MAX_BLANK_COUNT) start_step=-1; 
				if(start_blank<=0) start_step=1;
			}			
		}
		
	}
}