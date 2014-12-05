using UnityEngine;
using System.Collections;
using com.thecodeway;

public class CallAxTrace : MonoBehaviour {
	private string buttonText = "Test";

	void OnGUI() {
		if(GUI.Button(new Rect(100, 200, 400, 100), buttonText)) {
			AxTrace.SetTraceServer("192.168.31.152", 1978);

			AxTrace.Trace("-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
			AxTrace.Trace("中文字符+Ascii");
			AxTrace.Trace("MultiLineTest\nLine{0}:第一行\nLine2:{1}\nLine3:第三行pi={2}", 1, "第二行", 3.14f);
			
			AxTrace.TraceEx(0, 1, "AxTraceExA: 中文字符(CPACP)");
			AxTrace.TraceEx(0, 2, "AxTraceExA: 中文字符(UTF8)");
			AxTrace.TraceEx(1, 2, "Window1: AxTraceExA: 中文字符(UTF8)");
			
			AxTrace.TraceEx(0, 3, "AxTraceExW: 中文字符");
			AxTrace.TraceEx(2, 3, "Window2: AxTraceExW: 中文字符");

			{
				int blank_Count=0;
				int step=1;
				int MAX_BLANK_COUNT=50;
				
				AxTrace.Trace("<BEGIN>");
				for(int i=0; i<500; i++)
				{
					byte[] szTemp = new byte[blank_Count + 1];
					
					int j=0;
					for(j=0; j<blank_Count; j++) szTemp[j]=(byte)' ';
					
					blank_Count+=step;
					if(blank_Count>=MAX_BLANK_COUNT) step=-1;
					if(blank_Count<=0)step=1;
					
					szTemp[j++] = (byte)'*';
					
					AxTrace.Trace(System.Text.Encoding.UTF8.GetString(szTemp));
				}
				AxTrace.Trace("<END>");
			}
			
			{
				//test value
				AxTrace.Value("Int_Test", (int)-12345);
				
				AxTrace.Value("BYTE_MAX_VALUE", (byte)byte.MaxValue);
				AxTrace.Value("BYTE_MIN_VALUE", (byte)byte.MinValue);

				AxTrace.Value("SBYTE_MAX_VALUE", (sbyte)sbyte.MaxValue);
				AxTrace.Value("SBYTE_MIN_VALUE", (sbyte)sbyte.MinValue);

				AxTrace.Value("SHORT_MAX_VALUE", (short)short.MaxValue);
				AxTrace.Value("SHORT_MIN_VALUE", (short)short.MinValue);
				
				AxTrace.Value("USHORT_MAX_VALUE", (ushort)ushort.MaxValue);
				AxTrace.Value("USHORT_MIN_VALUE", (ushort)ushort.MinValue);

				AxTrace.Value("INT_MAX_VALUE", (int)int.MaxValue);
				AxTrace.Value("INT_MIN_VALUE", (int)int.MinValue);
				
				AxTrace.Value("UINT_MAX_VALUE", (uint)uint.MaxValue);
				AxTrace.Value("UINT_MIN_VALUE", (uint)uint.MinValue);

				AxTrace.Value("LONG_MAX_VALUE", (long)long.MaxValue);
				AxTrace.Value("LONG_MIN_VALUE", (long)long.MinValue);
				
				AxTrace.Value("ULONG_MAX_VALUE", (ulong)ulong.MaxValue);
				AxTrace.Value("ULONG_MIN_VALUE", (ulong)ulong.MinValue);

				AxTrace.Value("Float_Test", (float)-3.1415926f);
				AxTrace.Value("Double_Test", (double)-3.141592653589793238462643383279);
				
				AxTrace.Value("FLOAT_MAX_VALUE", (float)float.MaxValue);
				AxTrace.Value("FLOAT_MIN_VALUE", (float)float.MinValue);
				
				AxTrace.Value("DOUBLE_MAX_VALUE", (double)double.MaxValue);
				AxTrace.Value("DOUBLE_MIN_VALUE", (double)double.MinValue);
				
				AxTrace.Value("String_Test", "String 汉字");
				
			}			
		}
	}
}
