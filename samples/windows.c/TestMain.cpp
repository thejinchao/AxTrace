#include <stdio.h>
#include <stdlib.h>
#include "ctpl.h"
#include "axtrace.win.h"

//--------------------------------------------------------------------------------------------
struct AxTraceRequest
{
	void operator()(int id)
	{
		axlog(AXT_TRACE, "[%d]%s", id, message.c_str());
	}

	std::string message;

	AxTraceRequest(const char* msg) : message(msg) {}
};

//--------------------------------------------------------------------------------------------
struct  AxValueRequest
{
	void operator()(int id)
	{
		axvalue(AXV_STR_UTF16, m_strName.c_str(), m_strValue.c_str());
	}

	std::string m_strName;
	std::wstring m_strValue;

	AxValueRequest(const char* szValueName, const wchar_t* wszVlaue) :m_strName(szValueName), m_strValue(wszVlaue) {}
};

double rand_number(double r1, double r2)
{
	double min = r1 < r2 ? r1 : r2;
	double range = abs(r1 - r2);
	return rand()*range / RAND_MAX + min;
}

//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	printf("============= AxTrace4 Test(C++) ================\n");
	srand((unsigned int)::time(0));

	//--------------------------
	{
		printf("AxTrace Test\n");
		system("pause");

		//test AxTrace
		axlog(AXT_TRACE, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
		axlog(AXT_TRACE, "中文字符+Ascii");
		axlog(AXT_TRACE, "MultiLineTest\nLine1:第一行\nLine2:第二行\nLine%d:第三行", 3);

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
		system("pause");

		int blank_Count = 0;
		int step = 1;
		int MAX_BLANK_COUNT = 50;

		axlog(AXT_TRACE, "<BEGIN>");
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

			axlog(AXT_TRACE, szTemp);
		}
		axlog(AXT_TRACE, "<END>");
	}
	
	//--------------------------
	{
		printf("AxTrace Multithread Pressure Test\n");
		system("pause");

		ctpl::thread_pool tp(std::thread::hardware_concurrency());

		int blank_Count = 0;
		int step = 1;
		int MAX_BLANK_COUNT = 50;

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

			tp.push(AxTraceRequest(szTemp));
		}

	}

	//--------------------------
	{
		printf("AxValue Test\n");
		system("pause");

		#define AXVALUE(name, ex, type, out_type, value) name=value; axvalue(type, #name ex, &name); printf(#name ex "=" out_type "\n", value);

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
		axvalue(AXV_STR_ACP, "String_ACP", pszString);

		const char* pszString_UTF8 = "String \xE6\xB1\x89\xE5\xAD\x97(UTF8)"; //String 汉字（utf8)
		axvalue(AXV_STR_UTF8, "String_UTF8", pszString_UTF8);

		const wchar_t* wszString = L"String 汉字(UTF16)";
		axvalue(AXV_STR_UTF16, "String_UTF16", wszString);

	}

	//--------------------------
	{
		printf("AxValue Pressure Test\n");
		system("pause");

		int start_blank = 0;
		int start_step = 1;
		int MAX_BLANK_COUNT = 50;

		for (int i = 0; i < 100; i++)
		{
			axvalue(AXV_INT32, "start_blank", &start_blank);

			int blank_Count = start_blank;
			int step = start_step;

			for (int j = 0; j < 50; j++)
			{
				wchar_t wszTemp[1024] = { 0 };

				int k = 0;
				for (k = 0; k < blank_Count; k++) wszTemp[k] = L' ';

				blank_Count += step;
				if (blank_Count >= MAX_BLANK_COUNT) step = -1;
				if (blank_Count <= 0)step = 1;

				wszTemp[k++] = L'*';
				wszTemp[k++] = 0;

				char value_name[32] = { 0 };
				_snprintf(value_name, 32, "Value_%02d", j);
				axvalue(AXV_STR_UTF16, value_name, wszTemp);
			}

			start_blank += start_step;
			if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
			if (start_blank <= 0) start_step = 1;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	{
		ctpl::thread_pool tp(std::thread::hardware_concurrency());

		int start_blank = 0;
		int start_step = 1;
		int MAX_BLANK_COUNT = 50;

		for (int i = 0; i<500; i++)
		{
			axvalue(AXV_INT32, "start_blank_multiThread", &start_blank);
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

				tp.push(AxValueRequest(value_name, wszTemp));
			}

			start_blank += start_step;
			if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
			if (start_blank <= 0) start_step = 1;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

	}

	{
		printf("AxMap2D Test\n");
		system("pause");

		const int ACTOR_COUNTS = 100;
		const int MOVE_COUNTS = 200;

		const double MAP_LEFT = -260.0;
		const double MAP_TOP = 256.0;
		const double MAP_RIGHT = 256.0;
		const double MAP_BOTTOM = -256.0;
		const double MOVE_STEP = 2.0;
		const double PI = 3.14159265;

		double MAP_WIDTH = MAP_RIGHT - MAP_LEFT;
		double MAP_HEIGHT = MAP_BOTTOM - MAP_TOP;

		struct Actor
		{
			double sx, sy;
			double tx, ty;

			double x, y;
			double dir;

			double current_distance(void) const {
				return sqrt((x - sx)*(x - sx) + (y - sy)*(y - sy));
			}
			double remain_distance(void) const {
				return sqrt((x - tx)*(x - tx) + (y - ty)*(y - ty));
			}
		};

		Actor allActors[ACTOR_COUNTS];
	
		std::function<void(Actor&)> _selectNextTarget = [&](Actor& actor) {
			do {
				actor.tx = rand_number(MAP_LEFT, MAP_RIGHT);
				actor.ty = rand_number(MAP_TOP, MAP_BOTTOM);
			} while (actor.remain_distance()< MOVE_STEP*10);
			actor.sx = actor.x;
			actor.sy = actor.y;
			actor.dir = atan2(actor.ty-actor.sy, actor.tx-actor.sx);
		};

		for (int i = 0; i < ACTOR_COUNTS; i++)
		{
			Actor& actor = allActors[i];

			actor.sx = actor.x = rand_number(MAP_LEFT, MAP_RIGHT);
			actor.sy = actor.y = rand_number(MAP_TOP, MAP_BOTTOM);
			_selectNextTarget(actor);
		}

		for (int i = 0; i < MOVE_COUNTS; i++)
		{
			ax2d_begin_scene("test", MAP_LEFT, MAP_TOP, MAP_RIGHT, MAP_BOTTOM, "{\"gridSize\":[32.0,32.0], \"gridPoint\":[-256.0, 256.0]}");

			for (int j = 0; j < ACTOR_COUNTS; j++)
			{
				Actor& actor = allActors[j];

				ax2d_actor("test", 100 + j, actor.x, actor.y, actor.dir, 0, nullptr);

				if (actor.remain_distance() <= MOVE_STEP) {
					_selectNextTarget(actor);
				}

				if ((rand() % 10) > 5) {
					double distance = actor.current_distance() + MOVE_STEP;
					actor.x = actor.sx + distance * cos(actor.dir);
					actor.y = actor.sy + distance * sin(actor.dir);
				}
			}
			ax2d_end_scene("test");
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		system("pause");

		for (int i = 0; i < ACTOR_COUNTS; i++)
		{
			char temp[256] = { 0 };
			double gridPointX = -256.0;
			double gridPointY = 2560;
			_snprintf(temp, 256, "{\"gridSize\":[32.0,32.0], \"gridPoint\":[%f,%f]}", gridPointX+i, gridPointY-i);

			ax2d_begin_scene("test", MAP_LEFT * 2, MAP_TOP * 2, MAP_RIGHT * 2, MAP_BOTTOM * 2, temp);
			for (int j = 0; j < ACTOR_COUNTS-i; j++)
			{
				Actor& actor = allActors[j];
				ax2d_actor("test", 100 + j, actor.x, actor.y, actor.dir, 0, nullptr);
			}
			ax2d_end_scene("test");
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}


	}

	//--------------------------
	printf("DONE!\n");
	system("pause");
}

