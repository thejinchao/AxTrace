#include "StdAfx.h"
#include "AxTrace.h"
#include "TestUtil.h"
#include "TestThreadPool.h"

//--------------------------------------------------------------------------------------------
class AxTraceRequest : public CThreadRequest
{
public:
	virtual void ThreadProc(unsigned int nThreadId)
	{
		AxTraceExW(0, 0, L"[%08x]%s", nThreadId, m_strTraceContent.c_str());
	}

private:
	std::wstring m_strTraceContent;

public:
	AxTraceRequest(const wchar_t* szContent) :m_strTraceContent(szContent) {}
	~AxTraceRequest();
};

//--------------------------------------------------------------------------------------------
class AxValueRequest : public CThreadRequest
{
public:
	virtual void ThreadProc(unsigned int nThreadId)
	{
		AxValue(m_strName.c_str(), AX_STR_UTF16, m_strValue.c_str());
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
		AxTrace("-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
		AxTrace("中文字符+Ascii");
		AxTrace("MultiLineTest\nLine1:第一行\nLine2:第二行\nLine%d:第三行",3);

		//test AxTraceExA
		AxTraceExA(0, 1, 0, "AxTraceExA: 中文字符(CPACP)");
		AxTraceExA(0, 2, 1, convertWideToUtf8(L"AxTraceExA: 中文字符(UTF8)").c_str());
		AxTraceExA(1, 2, 1, convertWideToUtf8(L"Window1: AxTraceExA: 中文字符(UTF8)").c_str());

		//test AxTraceExW
		AxTraceExW(0, 3, L"AxTraceExW: 中文字符");
		AxTraceExW(2, 3, L"Window2: AxTraceExW: 中文字符");
	}
	
	//--------------------------
	{
		printf("AxTrace Pressure Test\n");
		system("pause");

		int blank_Count=0;
		int step=1;
		int MAX_BLANK_COUNT=50;

		AxTrace("<BEGIN>");
		for(int i=0; i<500; i++)
		{
			wchar_t wszTemp[1024]={0};

			int j=0;
			for(j=0; j<blank_Count; j++) wszTemp[j]=L' ';

			blank_Count+=step;
			if(blank_Count>=MAX_BLANK_COUNT) step=-1;
			if(blank_Count<=0)step=1;

			wszTemp[j++] = L'*';
			wszTemp[j++]=0;

			AxTraceExW(0, 0, wszTemp);
		}
		AxTrace("<END>");
	}

	//--------------------------
	{
		printf("AxTrace Multithread Pressure Test\n");
		system("pause");
		CThreadPool threadPool(10);
		threadPool.Begin();

		int blank_Count=0;
		int step=1;
		int MAX_BLANK_COUNT=50;

		for(int i=0; i<500; i++)
		{
			wchar_t wszTemp[1024]={0};

			int j=0;
			for(j=0; j<blank_Count; j++) wszTemp[j]=L' ';

			blank_Count+=step;
			if(blank_Count>=MAX_BLANK_COUNT) step=-1;
			if(blank_Count<=0)step=1;

			wszTemp[j++] = L'*';
			wszTemp[j++]=0;

			threadPool.PushRequest(new AxTraceRequest(wszTemp));
			Sleep(1);
		}
	}

	//--------------------------
	{
		printf("AxValue Test\n");
		system("pause");

		#define AXVALUE(name, ex, type, out_type, value) name=value; AxValue(#name ex, type, &name); printf(#name ex "=" out_type "\n", value);

		char int_8=0;
		AXVALUE(int_8, "_0", AX_INT8, "%d", 0);
		AXVALUE(int_8, "_MIN", AX_INT8, "%d", SCHAR_MIN);
		AXVALUE(int_8, "_MAX", AX_INT8, "%d", SCHAR_MAX);

		unsigned int uint_8=0;
		AXVALUE(uint_8, "_MIN", AX_UINT8, "%d", 0);
		AXVALUE(uint_8, "_MAX", AX_UINT8, "%d", UCHAR_MAX);

		short int_16=0;
		AXVALUE(int_16, "_0", AX_INT16, "%d", 0);
		AXVALUE(int_16, "_MIN", AX_INT16, "%d", SHRT_MIN);
		AXVALUE(int_16, "_MAX", AX_INT16, "%d", SHRT_MAX);

		unsigned short uint_16=0;
		AXVALUE(uint_16, "_MIN", AX_UINT16, "%d", 0);
		AXVALUE(uint_16, "_MAX", AX_UINT16, "%d", USHRT_MAX);

		int int_32=8;
		AXVALUE(int_32, "_0", AX_INT32, "%d", 0);
		AXVALUE(int_32, "_MIN", AX_INT32, "%d", INT_MIN);
		AXVALUE(int_32, "_MAX", AX_INT32, "%d", INT_MAX);

		unsigned int uint_32=0;
		AXVALUE(uint_32, "_MIN", AX_UINT32, "%d", 0);
		AXVALUE(uint_32, "_MAX", AX_UINT32, "%u", UINT_MAX);

		__int64 int_64=0;
		AXVALUE(int_64, "_0",   AX_INT64, "%d", 0i64);
		AXVALUE(int_64, "_MIN", AX_INT64, "%I64d", LLONG_MIN);
		AXVALUE(int_64, "_MAX", AX_INT64, "%I64d", LLONG_MAX);

		unsigned __int64 uint_64=0;
		AXVALUE(uint_64, "_MIN", AX_UINT64, "%I64u", 0i64);
		AXVALUE(uint_64, "_MAX", AX_UINT64, "%I64u", ULLONG_MAX);

		float float_32=0.f;
		AXVALUE(float_32, "_0", AX_FLOAT32, "%f", 0.f);
		AXVALUE(float_32, "_MAX", AX_FLOAT32, "%e", FLT_MAX);
		AXVALUE(float_32, "_MIN", AX_FLOAT32, "%e", FLT_MIN);

		double double_64=0.0;
		AXVALUE(double_64, "_0", AX_FLOAT64, "%f", 0.0);
		AXVALUE(double_64, "_MAX", AX_FLOAT64, "%e", DBL_MAX);
		AXVALUE(double_64, "_MIN", AX_FLOAT64, "%e", DBL_MIN);

		const char* pszString = "String 汉字(ACP)";
		AxValue("String_ACP", AX_STR_ACP, pszString);

		const char* pszString_UTF8 = "String \xE6\xB1\x89\xE5\xAD\x97(UTF8)"; //String 汉字（utf8)
		AxValue("String_UTF8", AX_STR_UTF8, pszString_UTF8);

		const wchar_t* wszString = L"String 汉字(UTF16)";
		AxValue("String_UTF16", AX_STR_UTF16, wszString);

	}

	//--------------------------
	{
		printf("AxValue Pressure Test\n");
		system("pause");

		int start_blank=0;
		int start_step=1;
		int MAX_BLANK_COUNT=50;

		for(int i=0; i<100; i++)
		{
			AxValue("start_blank", AX_INT32, &start_blank);
			
			int blank_Count=start_blank;
			int step=start_step;

			for(int j=0; j<50; j++)
			{
				wchar_t wszTemp[1024]={0};

				int k=0;
				for(k=0; k<blank_Count; k++) wszTemp[k]=L' ';

				blank_Count+=step;
				if(blank_Count>=MAX_BLANK_COUNT) step=-1;
				if(blank_Count<=0)step=1;

				wszTemp[k++] = L'*';
				wszTemp[k++] = 0;

				char value_name[32]={0};
				_snprintf(value_name, 32, "Value_%02d", j);
				AxValue(value_name, AX_STR_UTF16, wszTemp);
			}

			start_blank += start_step;
			if(start_blank>=MAX_BLANK_COUNT) start_step=-1; 
			if(start_blank<=0) start_step=1;
			
			Sleep(10);
		}

		CThreadPool threadPool(10);
		threadPool.Begin();

		for(int i=0; i<500; i++)
		{
			AxValue("start_blank_multiThread", AX_INT32, &start_blank);
			int blank_Count=start_blank;
			int step=start_step;

			for(int j=0; j<50; j++)
			{
				wchar_t wszTemp[1024]={0};

				int k=0;
				for(k=0; k<blank_Count; k++) wszTemp[k]=L' ';

				blank_Count+=step;
				if(blank_Count>=MAX_BLANK_COUNT) step=-1;
				if(blank_Count<=0)step=1;

				wszTemp[k++] = L'*';
				wszTemp[k++] = 0;

				char value_name[32]={0};
				_snprintf(value_name, 32, "Value_%02d", j);


				threadPool.PushRequest(new AxValueRequest(value_name, wszTemp));
			}

			start_blank += start_step;
			if(start_blank>=MAX_BLANK_COUNT) start_step=-1; 
			if(start_blank<=0) start_step=1;
			
			Sleep(10);

		}

	}

	//--------------------------
	printf("DONE!\n");
	system("pause");
	return 0;
}
