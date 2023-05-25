using System;
using System.Threading.Tasks;
using System.Threading;
using System.Diagnostics;
using com.thecodeway;
using System.Numerics;

namespace AxTraceSample
{
	internal class Program
	{
		public static void AxTraceThread(object? obj)
		{
			if (obj != null)
			{
				AxTrace.Log(AxTrace.AXT_TRACE, (string)obj);
			}
		}

		private class AxValueParam
		{
			public string value_name;
			public string value;
			public AxValueParam(string _name, string _value)
			{
				value_name = _name;
				value = _value;
			}
		}
		public static void AxValueThread(object? obj)
		{
			if (obj != null) 
			{
				AxTrace.Value(AxTrace.AXT_TRACE,
					((AxValueParam)obj).value_name, ((AxValueParam)obj).value);
			}
		}

		public static double rand_number(Random rnd, double r1, double r2)
		{
			double min = r1 < r2 ? r1 : r2;
			double range = Math.Abs(r1 - r2);
			return rnd.NextDouble() * range + min;
		}

		public struct AxActor2D
		{
			public Int64 id;
			public uint type;
			public string info;
			public double speed;

			public double sx, sy;
			public double tx, ty;

			public double x, y;
			public double dir;

			public void init(uint index, double move_step)
			{
				id = 100 + index;
				type = index % 3;
				info = String.Format("actor({0})", id);
				speed = move_step * (type + 1);
			}
			public double current_distance()  
			{
				return Math.Sqrt((x - sx)*(x - sx) + (y - sy)*(y - sy));
			}
			public double remain_distance()  
			{
				return Math.Sqrt((x - tx)*(x - tx) + (y - ty)*(y - ty));
			}
			public void select_next_target(Random rnd, double map_left, double map_right, double map_top, double map_bottom, double move_step)
			{
				double min_distance = 10 * move_step;
				double max_distance = 100 * move_step;
				do
				{
					tx = rand_number(rnd, map_left, map_right);
					ty = rand_number(rnd, map_top, map_bottom);
				} while (remain_distance() < min_distance || remain_distance() > max_distance);
				sx = x;
				sy = y;
				dir = Math.Atan2(ty - sy, tx - sx);
			}
		}
		static void Main(string[] args)
		{
			System.Console.WriteLine("============= AxTrace3 Test(C#) ================\n");
			
			{
				AxTrace.Log(AxTrace.AXT_TRACE, "-=-=-=-=-=-= Hello,World -=-=-=-=-=-=-=-=-=-");
				AxTrace.Log(AxTrace.AXT_TRACE, "中文字符+Ascii");
				AxTrace.Log(AxTrace.AXT_TRACE, "MultiLineTest\nLine{0}:第一行\nLine2:{1}\nLine3:第三行pi={2}", 1, "第二行", 3.14f);

				AxTrace.Log(AxTrace.AXT_DEBUG, "DEBUG: This is a debug message");
				AxTrace.Log(AxTrace.AXT_INFO, "INFO: This is a info message");
				AxTrace.Log(AxTrace.AXT_WARN, "WARN: This is a warning message");
				AxTrace.Log(AxTrace.AXT_ERROR, "ERROR: This is a error message");
				AxTrace.Log(AxTrace.AXT_FATAL, "FATAL: This is a fatal message");
			}

			{
				Console.WriteLine("AxTrace Pressure Test\nPress any key to continue...");
				Console.ReadKey();

				int blank_Count = 0;
				int step = 1;
				int MAX_BLANK_COUNT = 50;

				AxTrace.Log(AxTrace.AXT_TRACE, "<BEGIN>");
				for (int i = 0; i < 500; i++)
				{
					byte[] szTemp = new byte[blank_Count + 1];

					int j = 0;
					for (j = 0; j < blank_Count; j++) szTemp[j] = (byte)' ';

					blank_Count += step;
					if (blank_Count >= MAX_BLANK_COUNT) step = -1;
					if (blank_Count <= 0) step = 1;

					szTemp[j++] = (byte)'*';

					string l = System.Text.Encoding.ASCII.GetString(szTemp);
					AxTrace.Log(AxTrace.AXT_TRACE, l);
				}
				AxTrace.Log(AxTrace.AXT_TRACE, "<END>");
			}

			{
				Console.WriteLine("AxTrace Multithread Pressure Test\nPress any key to continue...");
				Console.ReadKey();

				WaitCallback waitCallback = new WaitCallback(AxTraceThread);

				int blank_Count = 0;
				int step = 1;
				int MAX_BLANK_COUNT = 50;

				AxTrace.Log(AxTrace.AXT_TRACE, "<BEGIN>");
				for (int i = 0; i < 500; i++)
				{
					byte[] szTemp = new byte[blank_Count + 1];

					int j = 0;
					for (j = 0; j < blank_Count; j++) szTemp[j] = (byte)' ';

					blank_Count += step;
					if (blank_Count >= MAX_BLANK_COUNT) step = -1;
					if (blank_Count <= 0) step = 1;

					szTemp[j++] = (byte)'*';

					ThreadPool.QueueUserWorkItem(waitCallback,
						System.Text.Encoding.ASCII.GetString(szTemp));
				}
			}

			{
				Console.WriteLine("AxValue Test\nPress any key to continue...");
				Console.ReadKey();

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

			{
				Console.WriteLine("AxValue Pressure Test\nPress any key to continue...");
				Console.ReadKey();

				int start_blank = 0;
				int start_step = 1;
				int MAX_BLANK_COUNT = 50;

				for (int i = 0; i < 100; i++)
				{
					AxTrace.Value(AxTrace.AXT_TRACE, "start_blank", start_blank);

					int blank_Count = start_blank;
					int step = start_step;

					for (int j = 0; j < 50; j++)
					{
						byte[] szTemp = new byte[blank_Count + 1];

						int k = 0;
						for (k = 0; k < blank_Count; k++) szTemp[k] = (byte)' ';

						blank_Count += step;
						if (blank_Count >= MAX_BLANK_COUNT) step = -1;
						if (blank_Count <= 0) step = 1;

						szTemp[k++] = (byte)'*';

						string value_name = "Value_" + j;
						AxTrace.Value(AxTrace.AXT_TRACE, value_name,
							System.Text.Encoding.ASCII.GetString(szTemp));
					}

					start_blank += start_step;
					if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
					if (start_blank <= 0) start_step = 1;
					Thread.Sleep(10);
				}
			}

			{
				Console.WriteLine("AxValue Multithread Pressure Test\nPress any key to continue...");
				Console.ReadKey();

				int start_blank = 0;
				int start_step = 1;
				int MAX_BLANK_COUNT = 50;
				WaitCallback waitCallback = new WaitCallback(AxValueThread);

				for (int i = 0; i < 500; i++)
				{
					AxTrace.Value(AxTrace.AXT_TRACE, "start_blank_multiThread", start_blank);
					int blank_Count = start_blank;
					int step = start_step;

					for (int j = 0; j < 50; j++)
					{
						byte[] szTemp = new byte[blank_Count + 1];

						int k = 0;
						for (k = 0; k < blank_Count; k++) szTemp[k] = (byte)' ';

						blank_Count += step;
						if (blank_Count >= MAX_BLANK_COUNT) step = -1;
						if (blank_Count <= 0) step = 1;

						szTemp[k++] = (byte)'*';

						string value_name = "Value_" + j;

						AxValueParam axvp = new AxValueParam(String.Format("Value_{0}",j), System.Text.Encoding.ASCII.GetString(szTemp));
						ThreadPool.QueueUserWorkItem(waitCallback, axvp);
					}

					start_blank += start_step;
					if (start_blank >= MAX_BLANK_COUNT) start_step = -1;
					if (start_blank <= 0) start_step = 1;

					Thread.Sleep(10);
				}
			}
			
			{
				Console.WriteLine("Ax2DMap Test\nPress any key to continue...");
				Console.ReadKey();

				const int ACTOR_COUNTS = 100;
				const int MOVE_COUNTS = 500;

				const double MAP_LEFT = -260.0;
				const double MAP_TOP = 256.0;
				const double MAP_RIGHT = 256.0;
				const double MAP_BOTTOM = -256.0;
				const double MOVE_STEP = 1.0;
				Random rnd = new Random();

				AxActor2D[] allActors = new AxActor2D[ACTOR_COUNTS];

				AxTrace.Scene2DBegin("test", MAP_LEFT, MAP_TOP, MAP_RIGHT, MAP_BOTTOM, "{\"gridSize\":[32.0,32.0], \"gridPoint\":[-256.0, 256.0]}");
				for (uint i = 0; i < ACTOR_COUNTS; i++)
				{
					ref AxActor2D actor = ref allActors[i];
					actor.init(i, MOVE_STEP);

					actor.sx = actor.x = rand_number(rnd, MAP_LEFT, MAP_RIGHT);
					actor.sy = actor.y = rand_number(rnd, MAP_TOP, MAP_BOTTOM);
					actor.select_next_target(rnd, MAP_LEFT, MAP_RIGHT, MAP_TOP, MAP_BOTTOM, MOVE_STEP);
					AxTrace.Scene2DActor("test", actor.id, actor.x, actor.y, actor.dir, actor.type, actor.info);
				}

				AxTrace.Scene2DEnd("test");
				Console.WriteLine("Begin Random Move Test\nPress any key to continue...");
				Console.ReadKey();


				for (int i = 0; i < MOVE_COUNTS; i++)
				{
					AxTrace.Scene2DBegin("test", MAP_LEFT, MAP_TOP, MAP_RIGHT, MAP_BOTTOM, "{\"gridSize\":[32.0,32.0], \"gridPoint\":[-256.0, 256.0]}");

					for (int j = 0; j < ACTOR_COUNTS; j++)
					{
						ref AxActor2D actor = ref allActors[j];

						AxTrace.Scene2DActor("test", actor.id, actor.x, actor.y, actor.dir, actor.type, actor.info);

						if (actor.remain_distance() <= MOVE_STEP * 4)
						{
							actor.select_next_target(rnd, MAP_LEFT, MAP_RIGHT, MAP_TOP, MAP_BOTTOM, MOVE_STEP);

							string actorLog = String.Format("move to:{0}, {1}", actor.tx, actor.ty);

							AxTrace.Scene2DActorLog("test", actor.id, actorLog);
						}

						if (rnd.Next(10) > 2)
						{
							double distance = actor.current_distance() + actor.speed;
							actor.x = actor.sx + distance * Math.Cos(actor.dir);
							actor.y = actor.sy + distance * Math.Sin(actor.dir);
						}
					}
					AxTrace.Scene2DEnd("test");
					Thread.Sleep(10);
				}
				Console.WriteLine("Begin erase Actors.\nPress any key to continue...");
				Console.ReadKey();

				for (int i = 0; i <= ACTOR_COUNTS; i++)
				{
					double gridPointX = -256.0;
					double gridPointY = 256;

					AxTrace.Scene2DBegin("test", MAP_LEFT, MAP_TOP, MAP_RIGHT, MAP_BOTTOM, 
						String.Format("{{\"gridSize\":[32.0,32.0], \"gridPoint\":[{0}, {1}]}}", gridPointX + i, gridPointY - i));

					for (int j = 0; j < ACTOR_COUNTS-i; j++)
					{
						ref AxActor2D actor = ref allActors[j];

						AxTrace.Scene2DActor("test", actor.id, actor.x, actor.y, actor.dir, actor.type, actor.info);
					}
					AxTrace.Scene2DEnd("test");
					Thread.Sleep(100);
				}

				Console.WriteLine("Done!");
			}
		}
	}
}
