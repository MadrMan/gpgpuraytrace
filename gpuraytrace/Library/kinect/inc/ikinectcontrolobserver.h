#pragma once

template<typename T>
class IKinectControlObserver
{
public:
	virtual void Update(T data) = 0;
};