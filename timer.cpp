#include "timer.h"

#include <windows.h>

static __int64 GetFrequency() {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
}

struct Timer::Impl
{
	LARGE_INTEGER start_;
};


Timer::Timer()
	:
	pImpl_(new Impl())
{
	Start();
}

Timer::~Timer()
{
	delete pImpl_;
}

void Timer::Start()
{
	QueryPerformanceCounter(&pImpl_->start_);
}

__int64 Timer::Elapsed() const
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return now.QuadPart - pImpl_->start_.QuadPart;
}

double Timer::ElapsedSecond() const
{
	return Elapsed() / (double) GetFrequency();
}

