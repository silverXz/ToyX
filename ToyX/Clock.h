#pragma once

#include <windows.h>
#include <time.h>
#include <mmsystem.h>
#pragma comment(lib,"Winmm.lib")

namespace iv
{
	class Clock
	{
	public:
		static double GetCurrentTimeUS(void)
		{
			LARGE_INTEGER large_integer;
			QueryPerformanceFrequency(&large_integer);
			
			double fre = (double)large_integer.QuadPart;

			QueryPerformanceCounter(&large_integer);

			double ct = large_integer.QuadPart * 1000 / fre;
			return ct;
		}

		static double GetCurrentTimeMS(void)
		{
			return clock();
		}
	};

	class Timer
	{
	private:
		double mStart;
		double mFrequency;
	public:
		Timer() :mStart(0.0), mFrequency(0.0)
		{
			LARGE_INTEGER li;
			QueryPerformanceFrequency(&li);
			mFrequency = (double)li.QuadPart / 1000.0;
		}

		void Start()
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			mStart = (double)li.QuadPart;
		}

		double GetElapse()
		{
			LARGE_INTEGER li;
			QueryPerformanceCounter(&li);
			return ((double)li.QuadPart - mStart) / mFrequency;
		}
	};
}