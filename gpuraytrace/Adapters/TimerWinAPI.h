#pragma once

#include "../Common/Timer.h"

class TimerWinAPI : public Timer
{
public:
	TimerWinAPI();

	//! Get the current time in milliseconds
	virtual float getTime() override;

	//! Get a per-frame time based value
	virtual float getConstant() override;
	
	//! Update the timer, should be done once per frame
	virtual void update() override;

private:
	LARGE_INTEGER frequency;
	LARGE_INTEGER counter;
	float time;
	float constant;
};