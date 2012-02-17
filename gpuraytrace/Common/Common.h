#pragma once

#if defined(_WIN32)
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <d3d11.h>
	#include <D3DX11.h>
	#include <xnamath.h>
	#include <process.h>
#endif

#include <iostream>
#include <string>
#include <ostream>
#include <vector>
#include <fstream>
#include <cmath>

#define finline __forceinline

finline bool isnull(float f)
{
	return (std::fabs(f) < 0.00001f);
}