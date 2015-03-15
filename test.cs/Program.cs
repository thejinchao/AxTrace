using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using com.thecodeway;

namespace test.win32.cs
{
    class Program
    {
        static void Main(string[] args)
        {
            System.Console.WriteLine("============= AxTrace3 Test(C#) ================\n");

            {
                AxTrace.Trace(AxTrace.AXT_TRACE, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
                AxTrace.Trace(AxTrace.AXT_TRACE, "中文字符+Ascii");
                AxTrace.Trace(AxTrace.AXT_TRACE, "MultiLineTest\nLine{0}:第一行\nLine2:{1}\nLine3:第三行pi={2}", 1, "第二行", 3.14f);
            }

            {
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
            //TODO: remove this line
            Thread.Sleep(1000);
        }
    }
}
