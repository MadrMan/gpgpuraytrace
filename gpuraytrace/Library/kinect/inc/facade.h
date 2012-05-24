#pragma once
#include "iobserver.h"

class Kinect;
class Direction;

class Facade
{
public:
	__declspec(dllexport) Facade(void);
	__declspec(dllexport) ~Facade(void);

	__declspec(dllexport) void Start();
	__declspec(dllexport) void Stop();
	__declspec(dllexport) void Elevate(int angle);
	__declspec(dllexport) void CurrentElevation(int *angle);
	__declspec(dllexport) void AttachListener(IObserver<float *> *observer);
	__declspec(dllexport) void DetachListener(IObserver<float *> *observer);
private:
	Kinect *m_kinect;
	Direction *m_direction;
};