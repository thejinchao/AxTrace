# AxTrace
实时日志查看工具

![](http://thecodeway.com/blog/wp-content/uploads/2016/05/axtrace01.gif)

简单来说，AxTrace是给程序员在开发期间使用的一个日志工具，类似于[DebugView](https://technet.microsoft.com/en-us/sysinternals/debugview)，比如在程序中添加下面的语句：
```C++
//日志输出
axlog(AXT_TRACE, "Hello,World.");
axlog(AXT_ERROR, "This is error message!");
//变量输出
int v = 9527;
axvalue(AXT_TRACE, AXV_INT32, "TestValue", &v);
```
那么在AxTrace程序中就会显示出这条日志和变量值。

![](http://thecodeway.com/blog/wp-content/uploads/2016/05/axtrace02.gif)

和DebugView相比，AxTrace是专门为程序研发阶段而设计的日志工具，有一些很有用的特性：
* 跨平台，输出端支持**Windows**(C/C++)、**Linux**(C/C++)、**.NET**(C#)、**Android**(C#)、**JVM**(JAVA)、**Unity**(C#)、**Unreal4**(C++)后续还会再添加更多平台
* 提供axlog和axvalue两个主要输出函数，其中axlog用于输出日志，axvalue函数用于输出变量值，随时将所需要打印的变量值输出到窗口的固定位置
* 线程安全，无阻塞，最大程度减少对发送程序的效率影响
* 使用TCP/IP协议，支持远程接收日志
* 多窗口显示日志
* 使用由LUA脚本控制的Filter模块，灵活设置日志是否抛弃，显示的窗口、字体颜色、背景色等信息
* 发送端不需要链接额外的库，只需要将几个文件添加到工程中即可，使用简单

AxTrace的第一版出现在十几年前，当时我在写一个网络程序，发现用OutputDebugString输出日志很不方便，首先整个系统的日志信息混杂到一个窗口里，另外Filter太简陋，有用和无用的信息混杂到一起，于是就有了第一版AxTrace，当时因为都是在Windows平台上，就直接使用WM_COPYDATA实现跨进程的消息传递，在写《天龙八部》时，写了第二版AxTrace，完善了颜色定义和变量监控功能，但仍然使用WM_COPYDATA传输信息，最近几年在写移动平台程序时，将底层传输改为TCP/IP协议，并且实现了多个平台上的发送端代码。

对于日志程序来说，过滤器是非常关键的功能，AxTrace最初的设计是固定提供几种日志类型，比如TRACE、DEBUG、INFO、WARN、ERROR、FATAL，在程序中设置这几种日志的颜色和是否显示，后来发现在不同场合对过滤器的要求都不一样，比如说下面几种情况
* 在调试一个C/S程序时，需要把服务器日志和客户端日志输出到一个窗口中
* 在研发某个功能时，要求把日志信息中含有“foo”字符串的日志高亮显示
* 在研发某个多线程程序时，要求所有日志分线程显示在独立的窗口

AxTrace为了实现这些功能，使用了LUA脚本来做过滤器，以缺省的过滤器脚本为例
```LUA
function onTraceMessage(msg) 
 local frontColor=COL_BLACK; 
 local backColor=COL_WHITE; 
 local msgStyle=msg:get_style(); 
 if(msgStyle==AXT_ERROR) then 
   frontColor=COL_RED; 
 end; 
 if(msgStyle==AXT_FATAL) then 
   frontColor=COL_RED; 
   backColor=COL_YELLOW; 
 end; 
 -- 四个参数分别是 是否显示，窗口标题，字体颜色，背景颜色
 return true, ("default"), frontColor, backColor; 
end; 
```
这段代码的意思是，所有日志信息输出到标题为”default”的日志窗口中，黑字白底，ERROR、和FATAL类型的消息使用特殊颜色标记。msg:get_style()用来获取日志消息的类型，也就是axlog函数的第一个参数，这是一个整数类型参数，除了预定义的TRACE、DEBUG、INFO、WARN、ERROR、FATAL之外，你可以任意使用其他自定义的数以实现复杂的过滤器功能。另外msg还有几个函数可以使用
```LUA
-- 获取进程ID
local pid = msg:get_process_id()
-- 获取线程ID
local tid = msg:get_thread_id()
-- 获取日志类型
local style = msg:get_style();
-- 获取日志内容
local content = msg:get_content();
```
后面AxTrace仍会继续开发，比如支持IOS程序，另外还有一个重要的功能，就是ax2d系列函数加入，用来以图像形式显示程序中的位置信息，这在开发MMORPG游戏服务器时非常有用，可以用图像的方式实时监控服务器程序中玩家和NPC的位置信息。

## 如何编译
### 编译Cyclone
AxTrace使用了我的另外一个开源工程cyclone作为网络底层，使用如下方法编译并安装该库

1.  创建一个空的目录作为cyclone根目录，例如"d:/cyclone"
2.  将cyclone源码clone到该目录下的一个子目录中，例如"d:/cyclone/src"，使用命令行为
``` git clone https://github.com/thejinchao/cyclone.git src ```
3.  在根目录下创建一个工程目录用来作为编译使用，例如"d:/cyclone/_sln"，在该目录中使用cmake生成工程文件，例如下面的命令行
``` cmake -G "Visual Studio 14 2015" ../src  ```
4.  打开生成的工程文件，编译cyclone，并执行其中的install工程，则"d:/cyclone/sdk"目录则会安装编译之后的cyclone，设置环境变量**CYCLONE_SDK_ROOT**到该目录

### 编译AxTrace
1. 创建一个空目录作为AxTrace的根目录，例如"d:/AxTrace"
2. 将AxTrace源码clone到该目录下的一个子目录中，例如"d:/AxTrace/src"
3. 在AxTrace根目录下创建一个子目录作为工程目录，例如"d:/AxTrace/_sln"
4. 在工程目录中使用cmake生成工程文件，命令行为```cmake -G "Visual Studio 14 2015" ../src ```
5. 打开工程文件并编译

