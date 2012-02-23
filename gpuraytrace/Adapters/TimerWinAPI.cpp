#include <Common.h>
#include "TimerWinAPI.h"

TimerWinAPI::TimerWinAPI()
{
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	time = 0.0f;
	constant = 0.0f;
}

float TimerWinAPI::getTime()
{
	return time;
}

float TimerWinAPI::getConstant()
{
	return constant;
}

void TimerWinAPI::update()
{
	const static int precision = 100000;
	float freq = (float)(frequency.QuadPart / precision);
	QueryPerformanceCounter(&counter);
	float oldTime = time;
	time = (counter.QuadPart / freq) / (float)precision;
	constant = time - oldTime;
}