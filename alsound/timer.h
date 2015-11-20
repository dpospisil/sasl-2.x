#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef WIN32
#include <windows.h>
#endif
#ifdef LIN
#define _POSIX_C_SOURCE 199309L
#include <sys/time.h>
#include <time.h>
#endif
#ifdef APL
#include <mach/mach_time.h>
#include <stdint.h>
#endif

#if defined(LIN) || defined(APL)
#define TIMER_NANO_DOUBLE_MULT 1000000000.0
#endif

namespace timer {

	class Timer {
	public:
		Timer();
		~Timer();
		void start();
		void stop();
		double getElapsedSec() const;
	private:
		void initFrequency();
		void createTimeStamp(double&) const;
	private:
		double mStartPoint;
		double mStopPoint;
		static bool mIsFrequencyInitialized;
		static bool mIsTimerWorks;
		bool mIsStarted;
		static double mFrequencyCoeff;
	private:
		Timer(const Timer&);
		Timer& operator=(const Timer&);
	};

}

#endif // __TIMER_H__

