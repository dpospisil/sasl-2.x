#include "timer.h"

namespace timer {

	bool Timer::mIsFrequencyInitialized = false;
	bool Timer::mIsTimerWorks = false;
	double Timer::mFrequencyCoeff = 1.0;

	Timer::Timer() {
		mIsStarted = false;
		mStartPoint = 0.0;
		mStopPoint = 0.0;
	}

	Timer::~Timer() {}

	void Timer::initFrequency() {
#ifdef WIN32
		LARGE_INTEGER uLI;
		if (!QueryPerformanceFrequency(&uLI)) {
			mIsFrequencyInitialized = true;
			mIsTimerWorks = false;
			return;
		} else {
			mFrequencyCoeff = (double)(uLI.QuadPart);
			mIsTimerWorks = true;
		}
#endif
#ifdef LIN
		mFrequencyCoeff = 1.0;
#endif
#ifdef APL
		mach_timebase_info_data_t timebase = { 0 };
		mach_timebase_info(&timebase);
		mFrequencyCoeff = (double)timebase.numer /
			(double)timebase.denom / TIMER_NANO_DOUBLE_MULT;
#endif
		mIsTimerWorks = true;
		mIsFrequencyInitialized = true;
	}

	void Timer::start() {
		if (!mIsFrequencyInitialized) {
			initFrequency();
		}
		createTimeStamp(mStartPoint);
		mIsStarted = true;
	}

	void Timer::stop() {
		createTimeStamp(mStopPoint);
		mIsStarted = false;
	}

	void Timer::createTimeStamp(double& outTimePoint) const {
#ifdef WIN32
		LARGE_INTEGER uLI;
		QueryPerformanceCounter(&uLI);
		outTimePoint = uLI.QuadPart / mFrequencyCoeff;
#endif
#ifdef LIN
		timespec timepoint;
		clock_gettime(CLOCK_MONOTONIC, &timepoint);
		outTimePoint = (double)timepoint.tv_sec +
			(double)timepoint.tv_nsec / TIMER_NANO_DOUBLE_MULT;
#endif
#ifdef APL
		outTimePoint = mach_absolute_time() * mFrequencyCoeff;
#endif
	}

	double Timer::getElapsedSec() const {
		if (!mIsStarted) {
			return mStopPoint - mStartPoint;
		} else {
			double currentTimePoint;
			createTimeStamp(currentTimePoint);
			return currentTimePoint - mStartPoint;
		}
	}

}