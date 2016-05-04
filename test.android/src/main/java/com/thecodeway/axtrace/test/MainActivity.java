package com.thecodeway.axtrace.test;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

import com.thecodeway.axtrace.AxTrace;

public class MainActivity extends Activity implements OnClickListener
{
	private Button buttonTest;
	private TextView editServerIp;
	private TextView editServerPort;
	private int currentStep;
	
	private ExecutorService threadPool = Executors.newFixedThreadPool(5);
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        buttonTest = (Button)findViewById(R.id.button_test);
        buttonTest.setOnClickListener(this);
        
        editServerIp = (TextView)findViewById(R.id.edit_server_ip);
        editServerPort = (TextView)findViewById(R.id.edit_server_port);
        
        currentStep = 0;
        
        //set axtrace server address
        //AxTrace.SetTraceServer("127.0.0.1", 1978);
    }
    
    @Override
    public void onClick(View v) {
    	switch(v.getId()) {
    	case R.id.button_test:
    		startTest();
    		break;
    	}
    }
    
    private void startTest() {
    	String serverIp = editServerIp.getText().toString();
    	String serverPort = editServerPort.getText().toString();
    	AxTrace.SetTraceServer(serverIp, (int)Long.parseLong(serverPort));
 
  	
		//test AxTrace
		if(currentStep==0) {
    	AxTrace.Trace(AxTrace.AXT_TRACE, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
    	AxTrace.Trace(AxTrace.AXT_TRACE, "中文字符+Ascii");
    	AxTrace.Trace(AxTrace.AXT_TRACE, "MultiLineTest\nLine1:第一行\nLine2:第二行\nLine%d:第三行",3);
    	AxTrace.Trace(AxTrace.AXT_WARN, "警告消息");
    	AxTrace.Trace(AxTrace.AXT_ERROR, "This error message");
    	AxTrace.Trace(AxTrace.AXT_FATAL, "This fatal message");
    	
    	currentStep++;
    	buttonTest.setText("Pressure Test");
    	return;
    }
    	
		//--------------------------
		//pressure test
		if(currentStep==1) {
			int blank_Count=0;
			int step=1;
			int MAX_BLANK_COUNT=50;

			AxTrace.Trace(AxTrace.AXT_TRACE, "<BEGIN>");
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
				AxTrace.Trace(AxTrace.AXT_TRACE, str);
			}
			AxTrace.Trace(AxTrace.AXT_TRACE, "<END>");

    	currentStep++;
    	buttonTest.setText("Multithread Pressure Test");
    	return;
		}

    	
		//--------------------------------------
		//thread pool press test
		if(currentStep==2) {
			class TraceThread extends Thread {
				public TraceThread(String str) {
					strToShow = str;
				}
				private String strToShow;
				public void run() {
					AxTrace.Trace(AxTrace.AXT_TRACE, strToShow);
                }
			}
			
			int blank_Count=0;
			int step=1;
			int MAX_BLANK_COUNT=50;
			AxTrace.Trace(AxTrace.AXT_TRACE, "<MULTI THREAD BEGIN>");
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
			
			AxTrace.Trace(AxTrace.AXT_TRACE, "<MULTI THREAD END>");

    	currentStep++;
    	buttonTest.setText("Value Test");
    	return;
		}
		
    	
    	
		//--------------------------------------
		//value test
		if(currentStep==3) {
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

    	currentStep++;
    	buttonTest.setText("Value Pressure Test");
    	return;
		}
		
    			
		if(currentStep==4) {

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

    	currentStep++;
    	buttonTest.setText("Value Multithread Pressure Test");
    	return;
		}
		
		
		if(currentStep==5) {
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

    	currentStep++;
    	buttonTest.setText("Done");
    	return;
		}
	}
}
