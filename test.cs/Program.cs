using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Diagnostics;
using com.thecodeway;

namespace test.win32.cs
{
    class Program
    {
        public static void AxTraceThread(object obj)
        {
            AxTrace.Trace(AxTrace.AXT_TRACE, (string)obj);
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

        static void Main(string[] args)
        {
            System.Console.WriteLine("============= AxTrace3 Test(C#) ================\n");
            
            {
                AxTrace.Trace(AxTrace.AXT_TRACE, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
                AxTrace.Trace(AxTrace.AXT_TRACE, "中文字符+Ascii");
                AxTrace.Trace(AxTrace.AXT_TRACE, "MultiLineTest\nLine{0}:第一行\nLine2:{1}\nLine3:第三行pi={2}", 1, "第二行", 3.14f);

                AxTrace.Trace(AxTrace.AXT_DEBUG, "DEBUG: This is a debug message");
                AxTrace.Trace(AxTrace.AXT_INFO, "INFO: This is a info message");
                AxTrace.Trace(AxTrace.AXT_WARN, "WARN: This is a warning message");
                AxTrace.Trace(AxTrace.AXT_ERROR, "ERROR: This is a error message");
                AxTrace.Trace(AxTrace.AXT_FATAL, "FATAL: This is a fatal message");
            }


            {
                Console.WriteLine("AxTrace Pressure Test\nPress any key to continue...");
                Console.ReadKey();

		        int blank_Count = 0;
		        int step = 1;
		        int MAX_BLANK_COUNT = 50;

		        AxTrace.Trace(AxTrace.AXT_TRACE, "<BEGIN>");
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
			        AxTrace.Trace(AxTrace.AXT_TRACE, l);
		        }
                AxTrace.Trace(AxTrace.AXT_TRACE, "<END>");
            }
           
            {
                Console.WriteLine("AxTrace Multithread Pressure Test\nPress any key to continue...");
                Console.ReadKey();

                WaitCallback waitCallback = new WaitCallback(AxTraceThread);

                int blank_Count = 0;
                int step = 1;
                int MAX_BLANK_COUNT = 50;

                AxTrace.Trace(AxTrace.AXT_TRACE, "<BEGIN>");
                for (int i = 0; i < 500; i++)
                {
                    byte[] szTemp = new byte[blank_Count + 1];

                    int j = 0;
                    for (j = 0; j < blank_Count; j++) szTemp[j] = (byte)' ';

                    blank_Count += step;
                    if (blank_Count >= MAX_BLANK_COUNT) step = -1;
                    if (blank_Count <= 0) step = 1;

                    szTemp[j++] = (byte)'*';

                    ThreadPool.QueueUserWorkItem(waitCallback, 
                        System.Text.Encoding.ASCII.GetString(szTemp));
                }
            }
            

            {
                Console.WriteLine("AxValue Test\nPress any key to continue...");
                Console.ReadKey();

                //test value
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
            }

            {
                Console.WriteLine("AxValue Pressure Test\nPress any key to continue...");
                Console.ReadKey();

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
            }

            {
                Console.WriteLine("AxValue Multithread Pressure Test\nPress any key to continue...");
                Console.ReadKey();

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

				        szTemp[k++] = (byte)'*';

                        string value_name = "Value_" + j;

                        AxValueParam axvp = new AxValueParam();
                        axvp.value_name = "Value_" + j; ;
                        axvp.value = System.Text.Encoding.ASCII.GetString(szTemp);

                        ThreadPool.QueueUserWorkItem(waitCallback, axvp);
			        }

			        start_blank += start_step;
			        if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
			        if (start_blank <= 0) start_step = 1;

                    Thread.Sleep(10);
		        }
            }
        }
    }
}
