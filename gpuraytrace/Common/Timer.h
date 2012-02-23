#pragma once

class Timer
{
public:
	static Timer* get();

	//! Get the current time in milliseconds
	virtual float getTime() = 0;

	//! Get a per-frame time based value
	virtual float getConstant() = 0;
	
	//! Update the timer, should be done once per frame
	virtual void update() = 0;

protected:
	virtual ~Timer() { }

};