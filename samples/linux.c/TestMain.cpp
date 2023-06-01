﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "ctpl.h"
#include "axtrace.linux.h"

int press_any_key(const char* message)
{
    
	int c = 0;
	struct termios org_opts, new_opts;
	int res = 0;
    printf("%s\n", message);
	//-----  store old settings -----------
	res = tcgetattr(STDIN_FILENO, &org_opts);
	//---- set new terminal parms --------
	memcpy(&new_opts, &org_opts, sizeof(new_opts));
	new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
	c = getchar();
	//------  restore old settings ---------
	res = tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
	return c;
}

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
		axvalue(AXV_STR_UTF32, m_strName.c_str(), m_strValue.c_str());
	}

	std::string m_strName;
	std::wstring m_strValue;

	AxValueRequest(const char* szValueName, const wchar_t* wszVlaue) :m_strName(szValueName), m_strValue(wszVlaue) {}
};

//--------------------------------------------------------------------------------------------
double rand_number(double r1, double r2)
{
	double min = r1 < r2 ? r1 : r2;
	double range = abs(r1 - r2);
	return rand()*range / RAND_MAX + min;
}

//--------------------------------------------------------------------------------------------
struct AxActor2D
{
	int id;
	int type;
	char  info[64];
	double speed;

	double sx, sy;
	double tx, ty;

	double x, y;
	double dir;

	void init(int index, double move_step)
	{
		id = 100 + index;
		type = index % 3;
		snprintf(info, 32, "actor(%d)", id);
		speed = move_step * (type + 1);
	}
	double current_distance(void) const {
		return sqrt((x - sx)*(x - sx) + (y - sy)*(y - sy));
	}
	double remain_distance(void) const {
		return sqrt((x - tx)*(x - tx) + (y - ty)*(y - ty));
	}
	void select_next_target(double x_min, double x_max, double y_min, double y_max, double move_step)
	{
		double min_distance = 10 * move_step;
		double max_distance = 100 * move_step;

		do {
			tx = rand_number(x_min, x_max);
			ty = rand_number(y_min, y_max);
		} while (remain_distance() < min_distance || remain_distance() > max_distance);
		sx = x;
		sy = y;
		dir = atan2(ty - sy, tx - sx);
	};
};

//--------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    printf("============= AxTrace4 Linux Test(C++) ================\n");

    {
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
		press_any_key("AxTrace Pressure Test");

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
		press_any_key("AxTrace Multithread Pressure Test");

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
		press_any_key("AxValue Test");

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

		long long int_64 = 0;
		AXVALUE(int_64, "_0", AXV_INT64, "%lld", 0LL);
		AXVALUE(int_64, "_MIN", AXV_INT64, "%lld", LLONG_MIN);
		AXVALUE(int_64, "_MAX", AXV_INT64, "%lld", LLONG_MAX);

		unsigned long long uint_64 = 0;
		AXVALUE(uint_64, "_MIN", AXV_UINT64, "%llu", 0LL);
		AXVALUE(uint_64, "_MAX", AXV_UINT64, "%llu", ULLONG_MAX);

		float float_32 = 0.f;
		AXVALUE(float_32, "_0", AXV_FLOAT32, "%f", 0.f);
		AXVALUE(float_32, "_MAX", AXV_FLOAT32, "%e", FLT_MAX);
		AXVALUE(float_32, "_MIN", AXV_FLOAT32, "%e", FLT_MIN);

		double double_64 = 0.0;
		AXVALUE(double_64, "_0", AXV_FLOAT64, "%f", 0.0);
		AXVALUE(double_64, "_MAX", AXV_FLOAT64, "%e", DBL_MAX);
		AXVALUE(double_64, "_MIN", AXV_FLOAT64, "%e", DBL_MIN);

		const char* pszString_UTF8 = "String \xE6\xB1\x89\xE5\xAD\x97(UTF8)"; //String 汉字（utf8)
		axvalue(AXV_STR_UTF8, "String_UTF8", pszString_UTF8);

		const wchar_t* wszString = L"String 汉字(UTF32)";
		axvalue(AXV_STR_UTF32, "String_UTF32", wszString);
	}	
	
	//--------------------------
	{
		press_any_key("AxValue Pressure Test");

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
				snprintf(value_name, 32, "Value_%02d", j);
				axvalue(AXV_STR_UTF32, value_name, wszTemp);
			}

			start_blank += start_step;
			if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
			if (start_blank <= 0) start_step = 1;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}	
	
	{
	    press_any_key("AxValue Multi Thread Pressure Test");
	    
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
				snprintf(value_name, 32, "Value_%02d", j);

				tp.push(AxValueRequest(value_name, wszTemp));
			}

			start_blank += start_step;
			if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
			if (start_blank <= 0) start_step = 1;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

	}

	{
		press_any_key("AxMap2D Test");

		const int ACTOR_COUNTS = 100;
		const int MOVE_COUNTS = 200;

		const double MAP_X_MIN = -260.0;
		const double MAP_Y_MIN = -256.0;
		const double MAP_X_MAX = 256.0;
		const double MAP_Y_MAX = 256.0;
		const double MOVE_STEP = 2.0;
		const double PI = 3.14159265;

		double MAP_WIDTH = MAP_X_MAX - MAP_X_MIN;
		double MAP_HEIGHT = MAP_Y_MAX - MAP_Y_MIN;
		
		AxActor2D allActors[ACTOR_COUNTS];
	
		ax2d_begin_scene("test", MAP_X_MIN, MAP_Y_MIN, MAP_X_MAX, MAP_Y_MAX, "{\"gridSize\":[32.0,32.0], \"gridPoint\":[-256.0, 256.0]}");
		for (int i = 0; i < ACTOR_COUNTS; i++)
		{
			AxActor2D& actor = allActors[i];
			actor.init(i, MOVE_STEP);

			actor.sx = actor.x = rand_number(MAP_X_MIN, MAP_X_MAX);
			actor.sy = actor.y = rand_number(MAP_Y_MIN, MAP_Y_MAX);
			actor.select_next_target(MAP_X_MIN, MAP_X_MAX, MAP_Y_MIN, MAP_Y_MAX, MOVE_STEP);

			ax2d_actor("test", actor.id, actor.x, actor.y, actor.dir, actor.type, actor.info);
		}
		ax2d_end_scene("test");
		
		press_any_key("Begin Random Move Test");
		
		for (int i = 0; i < MOVE_COUNTS; i++)
		{
			ax2d_begin_scene("test", MAP_X_MIN, MAP_Y_MIN, MAP_X_MAX, MAP_Y_MAX, "{\"gridSize\":[32.0,32.0], \"gridPoint\":[-256.0, 256.0]}");

			for (int j = 0; j < ACTOR_COUNTS; j++)
			{
				AxActor2D& actor = allActors[j];

				ax2d_actor("test", actor.id, actor.x, actor.y, actor.dir, actor.type, actor.info);

				if (actor.remain_distance() <= MOVE_STEP*4) {
					actor.select_next_target(MAP_X_MIN, MAP_Y_MIN, MAP_X_MAX, MAP_Y_MAX, MOVE_STEP);

					char actor_log[128] = { 0 };
					snprintf(actor_log, 128, "move to:%f, %f", actor.tx, actor.ty);

					ax2d_actor_log("test", actor.id, actor_log);
				}

				if ((rand() % 10) > 2) {
					double distance = actor.current_distance() + actor.speed;
					actor.x = actor.sx + distance * cos(actor.dir);
					actor.y = actor.sy + distance * sin(actor.dir);
				}
			}
			ax2d_end_scene("test");
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		
		press_any_key("Remove Actor Test");

		for (int i = 0; i < ACTOR_COUNTS; i++)
		{
			char temp[256] = { 0 };
			double gridPointX = -256.0;
			double gridPointY = 2560;
			snprintf(temp, 256, "{\"gridSize\":[32.0,32.0], \"gridPoint\":[%f,%f]}", gridPointX+i, gridPointY-i);

			ax2d_begin_scene("test", MAP_X_MIN, MAP_Y_MIN, MAP_X_MAX, MAP_Y_MAX, temp);
			for (int j = 0; j < ACTOR_COUNTS-i; j++)
			{
				AxActor2D& actor = allActors[j];
				ax2d_actor("test", actor.id, actor.x, actor.y, actor.dir, actor.type, actor.info);
			}
			ax2d_end_scene("test");
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	
}
