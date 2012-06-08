#pragma once
#include "ikinectcontrolobserver.h"

class Kinect;
class Direction;

class __declspec(dllexport) KinectControl
{
public:
	KinectControl(void);
	~KinectControl(void);

	void Start();
	void Stop();
	void Elevate(int angle);
	const int CurrentElevation();
	const bool IsTracking();
	void AttachListener(IKinectControlObserver<float *> *observer);
	void DetachListener(IKinectControlObserver<float *> *observer);
private:
	Kinect *m_kinect;
	Direction *m_direction;
	bool m_started;
};