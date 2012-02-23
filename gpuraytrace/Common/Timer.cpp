#include <Common.h>
#include "Timer.h"

#include "../Adapters/TimerWinAPI.h"

Timer* Timer::get()
{
	static Timer* timer = nullptr;
	if(!timer)
	{
		timer = new TimerWinAPI();
	}
	return timer;
}