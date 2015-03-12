#include "StdAfx.h"
#include "TestUtil.h"
#include "TestThreadPool.h"

#include "axtrace.win.h"


//--------------------------------------------------------------------------------------------
class AxTraceRequest : public CThreadRequest
{
public:
	virtual void ThreadProc(unsigned int nThreadId)
	{
		axtrace(AXT_TRACE, "[%08x]%s", nThreadId, m_strTraceContent.c_str());
	}

private:
	std::string m_strTraceContent;

public:
	AxTraceRequest(const char* szContent) :m_strTraceContent(szContent) {}
	~AxTraceRequest();
};

//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	printf("============= AxTrace3 Test(C++) ================\n");

	//--------------------------
	{
		printf("AxTrace Test\n");
		system("pause");

		//test AxTrace
		axtrace(AXT_TRACE, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
		axtrace(AXT_TRACE, "中文字符+Ascii");
		axtrace(AXT_TRACE, "MultiLineTest\nLine1:第一行\nLine2:第二行\nLine%d:第三行", 3);

		//test axtrace type
		axtrace(AXT_DEBUG, "DEBUG: This is a debug message");
		axtrace(AXT_INFO, "INFO: This is a info message");
		axtrace(AXT_WARN, "WARN: This is a warning message");
		axtrace(AXT_ERROR, "ERROR: This is a error message");
		axtrace(AXT_FATAL, "FATAL: This is a fatal message");
	}

	//--------------------------
	{
		printf("AxTrace Pressure Test\n");
		system("pause");

		int blank_Count = 0;
		int step = 1;
		int MAX_BLANK_COUNT = 50;

		axtrace(AXT_TRACE, "<BEGIN>");
		for (int i = 0; i<500; i++)
		{
			char szTemp[1024] = { 0 };

			int j = 0;
			for (j = 0; j<blank_Count; j++) szTemp[j] = L' ';

			blank_Count += step;
			if (blank_Count >= MAX_BLANK_COUNT) step = -1;
			if (blank_Count <= 0)step = 1;

			szTemp[j++] = L'*';
			szTemp[j++] = 0;

			axtrace(AXT_TRACE, szTemp);
		}
		axtrace(AXT_TRACE, "<END>");
	}

	//--------------------------
	{
		printf("AxTrace Multithread Pressure Test\n");
		system("pause");
		CThreadPool threadPool(10, 1000);
		threadPool.Begin();

		int blank_Count = 0;
		int step = 1;
		int MAX_BLANK_COUNT = 50;

		for (int i = 0; i<500; i++)
		{
			char szTemp[1024] = { 0 };

			int j = 0;
			for (j = 0; j<blank_Count; j++) szTemp[j] = L' ';

			blank_Count += step;
			if (blank_Count >= MAX_BLANK_COUNT) step = -1;
			if (blank_Count <= 0)step = 1;

			szTemp[j++] = L'*';
			szTemp[j++] = 0;

			if (!threadPool.PushRequest(new AxTraceRequest(szTemp)))
			{
				axtrace(AXT_ERROR, "thread pool full!");
			}
			//Sleep(1);
		}

		threadPool.ShutDown();
	}

}

