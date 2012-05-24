#pragma once

template<typename T>
class IObserver
{
public:
	__declspec(dllexport) virtual void Update(T data) = 0;
};