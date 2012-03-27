#pragma once

#include "./Common/Types.h"

#if defined(_WIN32)
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <d3d11.h>
	#include <D3Dcompiler.h>
	#include <process.h>
	#include <WS2tcpip.h>
	#ifdef RT_HAS_DXSDK
	#include <xnamath.h>
	#else
	#include <DirectXMath.h>
	using namespace DirectX;
	#endif
#endif

#include <algorithm>
#include <iostream>
#include <string>
#include <ostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <ctime>
#include <map>
#include <sstream>

#define finline __forceinline

finline bool isnull(float f)
{
	return (std::fabs(f) < 0.00001f);
}

//! Callback base class
template<class T>
class ICallbackBase
{
public:
	virtual void run(T) = 0;
};

//! Generic callback class for callbacks with a single parameter
template<class T, class P>
class ICallback : public ICallbackBase<P>
{
public:
	typedef void (T::*Fptr)(P x);

	ICallback(T* obj, Fptr fptr) : obj(obj), fptr(fptr) 
	{ }

	virtual void run(P p) override
	{ (obj->*fptr)(p); }

private:
	T* obj;
	Fptr fptr;
};

//! Generic callback class for callbacks without any parameters
template<class T>
class ICallback<T, void> : public ICallbackBase<void>
{
public:
	typedef void (T::*Fptr)();

	ICallback(T* obj, Fptr fptr) : obj(obj), fptr(fptr) 
	{ }

	virtual void run() override
	{ (obj->*fptr)(); }

private:
	T* obj;
	Fptr fptr;
};

template<class A>
std::string convert(const A& value) 
{
	std::stringstream s;
	s << value;
	return s.str();
};

template<class A>
bool convert(const std::string& value, A* out) 
{
	std::stringstream s(value);
	s >> *out;
	return !s.fail();
};