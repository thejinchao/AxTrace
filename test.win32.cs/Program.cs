using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace test.win32.cs
{
    class Program
    {
        static void Main(string[] args)
        {
            System.Console.WriteLine("============= AxTrace3 Test(C#) ================\n");


            Axia.AxTrace(0, 0, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
            Axia.AxTrace(0, 0, "中文字符+Ascii");
            Axia.AxTrace(0, 0, "MultiLineTest\nLine1:第一行\nLine2:第二行\nLine3:第三行");


            System.Console.ReadLine();
        }
    }
}
