#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <float.h>

#include "axtrace.linux.h"

int main(int argc, char* argv[])
{
	printf("============= AxTrace3 Test(C++) ================\n");

	//--------------------------
	{
		printf("AxTrace Test\n");
		printf("Press Any Key To Continue\n");
		int r = getchar();

		//test AxTrace
		axlog(AXT_TRACE, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
		axlog(AXT_TRACE, "\xE4\xB8\xAD\xE6\x96\x87\xE5\xAD\x97\xE7\xAC\xA6+Ascii");	//utf8 ÖÐÎÄ×Ö·û
		axlog(AXT_TRACE, "MultiLineTest\nLine1:\xE7\xAC\xAC\xE4\xB8\x80\xE8\xA1\x8C\nLine2:\xE7\xAC\xAC\xE4\xBA\x8C\xE8\xA1\x8C\nLine%d:\xE7\xAC\xAC\xE4\xB8\x89\xE8\xA1\x8C", 3);

		//test axlog type
		axlog(AXT_DEBUG, "DEBUG: This is a debug message");
		axlog(AXT_INFO, "INFO: This is a info message");
		axlog(AXT_WARN, "WARN: This is a warning message");
		axlog(AXT_ERROR, "ERROR: This is a error message");
		axlog(AXT_FATAL, "FATAL: This is a fatal message");
	}
	
	//--------------------------
	{
		printf("AxTrace Pressure Test\n");
		printf("Press Any Key To Continue\n");
		int r = getchar();

		int blank_Count = 0;
		int step = 1;
		int MAX_BLANK_COUNT = 50;

		axlog(AXT_TRACE, "<BEGIN>");
		for (int i = 0; i < 500; i++)
		{
			char szTemp[1024] = { 0 };

			int j = 0;
			for (j = 0; j < blank_Count; j++) szTemp[j] = L' ';

			blank_Count += step;
			if (blank_Count >= MAX_BLANK_COUNT) step = -1;
			if (blank_Count <= 0)step = 1;

			szTemp[j++] = L'*';
			szTemp[j++] = 0;

			axlog(AXT_TRACE, szTemp);
		}
		axlog(AXT_TRACE, "<END>");
	}	
	
	//--------------------------
	{
		printf("AxValue Test\n");
		printf("Press Any Key To Continue\n");
		int r = getchar();

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

		int64_t int_64 = 0;
		AXVALUE(int_64, "_0", AXV_INT64, "%lld", 0ll);
		AXVALUE(int_64, "_MIN", AXV_INT64, "%lld", LLONG_MIN);
		AXVALUE(int_64, "_MAX", AXV_INT64, "%lld", LLONG_MAX);

		u_int64_t uint_64 = 0;
		AXVALUE(uint_64, "_MIN", AXV_UINT64, "%llu", 0ll);
		AXVALUE(uint_64, "_MAX", AXV_UINT64, "%llu", ULLONG_MAX);

		float float_32 = 0.f;
		AXVALUE(float_32, "_0", AXV_FLOAT32, "%f", 0.f);
		AXVALUE(float_32, "_MAX", AXV_FLOAT32, "%e", FLT_MAX);
		AXVALUE(float_32, "_MIN", AXV_FLOAT32, "%e", FLT_MIN);

		double double_64 = 0.0;
		AXVALUE(double_64, "_0", AXV_FLOAT64, "%f", 0.0);
		AXVALUE(double_64, "_MAX", AXV_FLOAT64, "%e", DBL_MAX);
		AXVALUE(double_64, "_MIN", AXV_FLOAT64, "%e", DBL_MIN);

		const char* pszString_UTF8 = "String \xE6\xB1\x89\xE5\xAD\x97(UTF8)"; //String ºº×Ö£¨utf8)
		axvalue(AXT_TRACE, AXV_STR_UTF8, "String_UTF8", pszString_UTF8);

		const char* wszString = "S\0t\0r\0i\0n\0g\0\x20\0\x49\x6C\x57\x5B\x28\0\x75\0\x74\0\x66\0\x31\0\x36\0\x29\0\0"; //String ºº×Ö£¨utf16)
		axvalue(AXT_TRACE, AXV_STR_UTF16, "String_UTF16", wszString);

	}	
	return 0;
}
