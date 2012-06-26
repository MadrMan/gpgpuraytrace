#pragma once

#include "./Common/Types.h"

#if defined(_WIN32)
	#include <Windows.h>
	#include <d3d11.h>
	#include <D3Dcompiler.h>
	#include <process.h>
	#include <WS2tcpip.h>
	#include <dxgiformat.h>
	#ifdef RT_HAS_DXSDK
	#include <xnamath.h>
	#else
	#include <DirectXMath.h>
	using namespace DirectX;
	#endif

	#define finline __forceinline
#else
	#define finline
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
#include <array>

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

template<>
class ICallbackBase<void>
{
public:
	virtual void run() = 0;
};

//! Generic callback class for callbacks with a single parameter
template<class T, class P>
class ICallback : public ICallbackBase<P>
{
public:
	//! Prototype for the function to call with the callback
	typedef void (T::*Fptr)(P x);

	//! Constructor
	//! \param obj Object to call the function with
	//! \param fptr Function to call when the callback fires
	ICallback(T* obj, Fptr fptr) : obj(obj), fptr(fptr) 
	{ }

	//Run the callback function
	//! \param p Argument to give to the function
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
	//! Prototype for the function to call with the callback
	typedef void (T::*Fptr)();

	//! Constructor
	//! \param obj Object to call the function with
	//! \param fptr Function to call when the callback fires
	ICallback(T* obj, Fptr fptr) : obj(obj), fptr(fptr) 
	{ }

	//Run the callback function
	virtual void run() override
	{ (obj->*fptr)(); }

private:
	T* obj;
	Fptr fptr;
};

//! Convert anything to a string
template<class A>
std::string convert(const A& value) 
{
	std::stringstream s;
	s << value;
	return s.str();
};

//! Convert a string to anything
//! \return Returns true on success, false if it could not be parsed to the specified type
template<class A>
bool convert(const std::string& value, A* out) 
{
	std::stringstream s(value);
	s >> *out;
	return !s.fail();
};
