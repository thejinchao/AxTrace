using UnityEngine;
using System.Collections;
using System.Threading;
using com.thecodeway;

public class MainCamera : MonoBehaviour {
	public static void AxTraceThread(object obj)
	{
	    AxTrace.Log(AxTrace.AXT_TRACE, (string)obj);
		//Debug.Log ("AxTraceThread:" + (string)obj);
	}
	
	private class AxValueParam
	{
	    public string value_name;
	    public string value;
	}
	public static void AxValueThread(object obj)
	{
	    AxTrace.Value(AxTrace.AXT_TRACE, 
	        ((AxValueParam)obj).value_name, ((AxValueParam)obj).value);
	}
	
	private string server_ip = "127.0.0.1";
	private string server_port = "1978";	
	private string buttonText = "Begin Test";
	private int m_lineHeight;

	void Start () 
	{
	#if UNITY_IPHONE || UNITY_ANDROID
		server_ip = "10.12.202.129";
	#endif
		Nyahoon.ThreadPool.InitInstance(128,0);
	}

	IEnumerator AxTraceTest() 
	{
		{
			AxTrace.Log(AxTrace.AXT_TRACE, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
			AxTrace.Log(AxTrace.AXT_TRACE, "中文字符+Ascii");
			AxTrace.Log(AxTrace.AXT_TRACE, "MultiLineTest\nLine{0}:第一行\nLine2:{1}\nLine3:第三行pi={2}", 1, "第二行", 3.14f);

			AxTrace.Log(AxTrace.AXT_DEBUG, "DEBUG: This is a debug message");
			AxTrace.Log(AxTrace.AXT_INFO, "INFO: This is a info message");
			AxTrace.Log(AxTrace.AXT_WARN, "WARN: This is a warning message");
			AxTrace.Log(AxTrace.AXT_ERROR, "ERROR: This is a error message");
			AxTrace.Log(AxTrace.AXT_FATAL, "FATAL: This is a fatal message");
			
			buttonText = "Pressure Test";
			yield return new WaitForSeconds(1f);
		}	
		
		{
			int blank_Count = 0;
			int step = 1;
			int MAX_BLANK_COUNT = 50;

			AxTrace.Log(AxTrace.AXT_TRACE, "<BEGIN>");
			for (int i = 0; i<500; i++)
			{
				byte[] szTemp = new byte[blank_Count+1];

				int j = 0;
				for (j = 0; j<blank_Count; j++) szTemp[j] = (byte)' ';

				blank_Count += step;
				if (blank_Count >= MAX_BLANK_COUNT) step = -1;
				if (blank_Count <= 0)step = 1;

				szTemp[j++] = (byte)'*';

				string l = System.Text.Encoding.ASCII.GetString(szTemp);
				AxTrace.Log(AxTrace.AXT_TRACE, l);
			}
			AxTrace.Log(AxTrace.AXT_TRACE, "<END>");
			
			buttonText = "Multithread Pressure Test";
			yield return new WaitForSeconds(1f);
		}
		{
			WaitCallback waitCallback = new WaitCallback(AxTraceThread);
			
			int blank_Count = 0;
			int step = 1;
			int MAX_BLANK_COUNT = 50;
			
			AxTrace.Log(AxTrace.AXT_TRACE, "<BEGIN>");
			for (int i = 0; i < 500; i++)
			{
			    byte[] szTemp = new byte[blank_Count + 1];
			
			    int j = 0;
			    for (j = 0; j < blank_Count; j++) szTemp[j] = (byte)' ';
			
			    blank_Count += step;
			    if (blank_Count >= MAX_BLANK_COUNT) step = -1;
			    if (blank_Count <= 0) step = 1;
			
			    szTemp[j++] = (byte)'#';
			
				Nyahoon.ThreadPool.QueueUserWorkItem(waitCallback, 
			        System.Text.Encoding.ASCII.GetString(szTemp));
			}
			
			buttonText = "AxValue Test Test";
			yield return new WaitForSeconds(1f);
		}
		{
			AxTrace.Value(AxTrace.AXT_TRACE, "Int_Test", (int)-12345);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "BYTE_MAX_VALUE", (byte)byte.MaxValue);
			AxTrace.Value(AxTrace.AXT_TRACE, "BYTE_MIN_VALUE", (byte)byte.MinValue);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "SHORT_MAX_VALUE", (short)short.MaxValue);
			AxTrace.Value(AxTrace.AXT_TRACE, "SHORT_MIN_VALUE", (short)short.MinValue);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "INT_MAX_VALUE", (int)int.MaxValue);
			AxTrace.Value(AxTrace.AXT_TRACE, "INT_MIN_VALUE", (int)int.MinValue);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "LONG_MAX_VALUE", (long)long.MaxValue);
			AxTrace.Value(AxTrace.AXT_TRACE, "LONG_MIN_VALUE", (long)long.MinValue);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "Float_Test", (float)-3.1415926f);
			AxTrace.Value(AxTrace.AXT_TRACE, "Double_Test", (double)-3.141592653589793238462643383279);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "FLOAT_MAX_VALUE", (float)float.MaxValue);
			AxTrace.Value(AxTrace.AXT_TRACE, "FLOAT_MIN_VALUE", (float)float.MinValue);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "DOUBLE_MAX_VALUE", (double)double.MaxValue);
			AxTrace.Value(AxTrace.AXT_TRACE, "DOUBLE_MIN_VALUE", (double)double.MinValue);
			
			AxTrace.Value(AxTrace.AXT_TRACE, "String_Test", "String 汉字 123");			
			buttonText = "AxValue Pressure Test";
			yield return new WaitForSeconds(1f);
		}
		{
			int start_blank = 0;
			int start_step = 1;
			int MAX_BLANK_COUNT = 50;
			
			for (int i = 0; i<100; i++)
			{
			    AxTrace.Value(AxTrace.AXT_TRACE, "start_blank", start_blank);
			
			    int blank_Count = start_blank;
			    int step = start_step;
			
			    for (int j = 0; j<50; j++)
			    {
			        byte[] szTemp = new byte[blank_Count+1];
			
			        int k = 0;
			        for (k = 0; k<blank_Count; k++) szTemp[k] = (byte)' ';
			
			        blank_Count += step;
			        if (blank_Count >= MAX_BLANK_COUNT) step = -1;
			        if (blank_Count <= 0)step = 1;
			
			        szTemp[k++] = (byte)'*';
			
			        string value_name = "Value_" + j;
			        AxTrace.Value(AxTrace.AXT_TRACE, value_name,
			            System.Text.Encoding.ASCII.GetString(szTemp));
			    }
			
			    start_blank += start_step;
			    if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
			    if (start_blank <= 0) start_step = 1;
			    Thread.Sleep(10);
			}
			
			buttonText = "AxValue MultiThread Pressure Test";
			yield return new WaitForSeconds(1f);
		}
		
		{
			int start_blank = 0;
			int start_step = 1;
			int MAX_BLANK_COUNT = 50;
			WaitCallback waitCallback = new WaitCallback(AxValueThread);
			
			for (int i = 0; i<500; i++)
			{
			    AxTrace.Value(AxTrace.AXT_TRACE, "start_blank_multiThread", start_blank);
			    int blank_Count = start_blank;
			    int step = start_step;
			
			    for (int j = 0; j<50; j++)
			    {
			        byte[] szTemp = new byte[blank_Count+1];
			
			        int k = 0;
			        for (k = 0; k<blank_Count; k++) szTemp[k] = (byte)' ';
			
			        blank_Count += step;
			        if (blank_Count >= MAX_BLANK_COUNT) step = -1;
			        if (blank_Count <= 0)step = 1;
			
			        szTemp[k++] = (byte)'#';
			
			        AxValueParam axvp = new AxValueParam();
			        axvp.value_name = "Value_" + j; ;
			        axvp.value = System.Text.Encoding.ASCII.GetString(szTemp);
			
					Nyahoon.ThreadPool.QueueUserWorkItem(waitCallback, axvp);
			    }
			
			    start_blank += start_step;
			    if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
			    if (start_blank <= 0) start_step = 1;
			
			    Thread.Sleep(10);
			}
			buttonText = "DONE!";
		}
	}

	void OnGUI() 
	{
	#if UNITY_IPHONE || UNITY_ANDROID
		
		int fontSize = 40;
		GUI.skin.button.fontSize = fontSize;
		GUI.skin.label.fontSize = fontSize;
		GUI.skin.textField.fontSize = fontSize;
		m_lineHeight = 80;
	#else
		m_lineHeight = 30;
	#endif
		
		int screenWidth = Screen.width;
		
		GUILayout.BeginHorizontal();  
		GUILayout.Label("IP", GUILayout.Width(200));
		server_ip = GUILayout.TextField(server_ip, GUILayout.Width(screenWidth-200), GUILayout.Height(m_lineHeight));
	    GUILayout.EndHorizontal();  
	    
		GUILayout.BeginHorizontal();  
		GUILayout.Label("PORT", GUILayout.Width(200));
		server_port = GUILayout.TextField(server_port, GUILayout.Width(screenWidth-200), GUILayout.Height(m_lineHeight));
	    GUILayout.EndHorizontal();  
				
		if(GUI.Button(new Rect(0, m_lineHeight*2, screenWidth, m_lineHeight), buttonText)) 
		{
			AxTrace.SetServer(server_ip, System.Int32.Parse(server_port));

			StartCoroutine(AxTraceTest());  
//			for(int i=0; i<10; i++){
//				string hello = "hello:" + i;
//				Nyahoon.ThreadPool.QueueUserWorkItem(new WaitCallback(AxTraceThread), hello);
//			}

			//Debug.Log ("main thread, tid=" + Thread.CurrentThread.ManagedThreadId);
			//for(int i=0; i<5; i++) {
			//	Thread newThread = new Thread(foo);
			//	newThread.Start ();
			//}
		}
	}	
}
