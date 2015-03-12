using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using com.thecodeway;

namespace test.win32.cs
{
    class Program
    {
        static void Main(string[] args)
        {
            System.Console.WriteLine("============= AxTrace3 Test(C#) ================\n");

            AxTrace.Trace(AxTrace.AXT_TRACE, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
            AxTrace.Trace(AxTrace.AXT_TRACE, "中文字符+Ascii");
            AxTrace.Trace(AxTrace.AXT_TRACE, "MultiLineTest\nLine{0}:第一行\nLine2:{1}\nLine3:第三行pi={2}", 1, "第二行", 3.14f);
        }
    }
}
