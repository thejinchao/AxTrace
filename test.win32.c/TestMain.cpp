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
class AxValueRequest : public CThreadRequest
{
public:
	virtual void ThreadProc(unsigned int nThreadId)
	{
		axvalue(AXT_TRACE, AXV_STR_UTF16, m_strName.c_str(), m_strValue.c_str());
	}

private:
	std::string m_strName;
	std::wstring m_strValue;

public:
	AxValueRequest(const char* szValueName, const wchar_t* wszVlaue) :m_strName(szValueName), m_strValue(wszVlaue) {}
	~AxValueRequest();
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

	//--------------------------
	{
		printf("AxValue Test\n");
		system("pause");

		#define AXVALUE(name, ex, type, out_type, value) name=value; axvalue(AXT_TRACE, type, #name ex, &name); printf(#name ex "=" out_type "\n", value);

		char int_8 = 0;
		AXVALUE(int_8, "_0", AXV_INT8, "%d", 0);
		AXVALUE(int_8, "_MIN", AXV_INT8, "%d", SCHAR_MIN);
		AXVALUE(int_8, "_MAX", AXV_INT8, "%d", SCHAR_MAX);

		unsigned int uint_8 = 0;
		AXVALUE(uint_8, "_MIN", AXV_UINT8, "%d", 0);
		AXVALUE(uint_8, "_MAX", AXV_UINT8, "%d", UCHAR_MAX);

		short int_16 = 0;
		AXVALUE(int_16, "_0", AXV_INT16, "%d", 0);
		AXVALUE(int_16, "_MIN", AXV_INT16, "%d", SHRT_MIN);
		AXVALUE(int_16, "_MAX", AXV_INT16, "%d", SHRT_MAX);

		unsigned short uint_16 = 0;
		AXVALUE(uint_16, "_MIN", AXV_UINT16, "%d", 0);
		AXVALUE(uint_16, "_MAX", AXV_UINT16, "%d", USHRT_MAX);

		int int_32 = 8;
		AXVALUE(int_32, "_0", AXV_INT32, "%d", 0);
		AXVALUE(int_32, "_MIN", AXV_INT32, "%d", INT_MIN);
		AXVALUE(int_32, "_MAX", AXV_INT32, "%d", INT_MAX);

		unsigned int uint_32 = 0;
		AXVALUE(uint_32, "_MIN", AXV_UINT32, "%d", 0);
		AXVALUE(uint_32, "_MAX", AXV_UINT32, "%u", UINT_MAX);

		__int64 int_64 = 0;
		AXVALUE(int_64, "_0", AXV_INT64, "%I64d", 0i64);
		AXVALUE(int_64, "_MIN", AXV_INT64, "%I64d", LLONG_MIN);
		AXVALUE(int_64, "_MAX", AXV_INT64, "%I64d", LLONG_MAX);

		unsigned __int64 uint_64 = 0;
		AXVALUE(uint_64, "_MIN", AXV_UINT64, "%I64u", 0i64);
		AXVALUE(uint_64, "_MAX", AXV_UINT64, "%I64u", ULLONG_MAX);

		float float_32 = 0.f;
		AXVALUE(float_32, "_0", AXV_FLOAT32, "%f", 0.f);
		AXVALUE(float_32, "_MAX", AXV_FLOAT32, "%e", FLT_MAX);
		AXVALUE(float_32, "_MIN", AXV_FLOAT32, "%e", FLT_MIN);

		double double_64 = 0.0;
		AXVALUE(double_64, "_0", AXV_FLOAT64, "%f", 0.0);
		AXVALUE(double_64, "_MAX", AXV_FLOAT64, "%e", DBL_MAX);
		AXVALUE(double_64, "_MIN", AXV_FLOAT64, "%e", DBL_MIN);

		const char* pszString = "String 汉字(ACP)";
		axvalue(AXT_TRACE, AXV_STR_ACP, "String_ACP", pszString);

		const char* pszString_UTF8 = "String \xE6\xB1\x89\xE5\xAD\x97(UTF8)"; //String 汉字（utf8)
		axvalue(AXT_TRACE, AXV_STR_UTF8, "String_UTF8", pszString_UTF8);

		const wchar_t* wszString = L"String 汉字(UTF16)";
		axvalue(AXT_TRACE, AXV_STR_UTF16, "String_UTF16", wszString);

	}

	//--------------------------
	{
		printf("AxValue Pressure Test\n");
		system("pause");

		int start_blank = 0;
		int start_step = 1;
		int MAX_BLANK_COUNT = 50;

		for (int i = 0; i<100; i++)
		{
			axvalue(AXT_TRACE, AXV_INT32, "start_blank", &start_blank);

			int blank_Count = start_blank;
			int step = start_step;

			for (int j = 0; j<50; j++)
			{
				wchar_t wszTemp[1024] = { 0 };

				int k = 0;
				for (k = 0; k<blank_Count; k++) wszTemp[k] = L' ';

				blank_Count += step;
				if (blank_Count >= MAX_BLANK_COUNT) step = -1;
				if (blank_Count <= 0)step = 1;

				wszTemp[k++] = L'*';
				wszTemp[k++] = 0;

				char value_name[32] = { 0 };
				_snprintf(value_name, 32, "Value_%02d", j);
				axvalue(AXT_TRACE, AXV_STR_UTF16, value_name, wszTemp);
			}

			start_blank += start_step;
			if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
			if (start_blank <= 0) start_step = 1;

			Sleep(10);
		}

		CThreadPool threadPool(10);
		threadPool.Begin();

		for (int i = 0; i<500; i++)
		{
			axvalue(AXT_TRACE, AXV_INT32, "start_blank_multiThread", &start_blank);
			int blank_Count = start_blank;
			int step = start_step;

			for (int j = 0; j<50; j++)
			{
				wchar_t wszTemp[1024] = { 0 };

				int k = 0;
				for (k = 0; k<blank_Count; k++) wszTemp[k] = L' ';

				blank_Count += step;
				if (blank_Count >= MAX_BLANK_COUNT) step = -1;
				if (blank_Count <= 0)step = 1;

				wszTemp[k++] = L'*';
				wszTemp[k++] = 0;

				char value_name[32] = { 0 };
				_snprintf(value_name, 32, "Value_%02d", j);


				threadPool.PushRequest(new AxValueRequest(value_name, wszTemp));
			}

			start_blank += start_step;
			if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
			if (start_blank <= 0) start_step = 1;

			Sleep(10);

		}

	}

	//--------------------------
	printf("DONE!\n");
	system("pause");
}

