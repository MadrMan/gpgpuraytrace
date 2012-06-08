#pragma once

template<typename T>
class IKinectControlObserver
{
public:
	virtual ~IKinectControlObserver() {}
	virtual void Update(T data) = 0;
};