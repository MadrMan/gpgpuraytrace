#pragma once

#include "IWindow.h"

class WindowFactory
{
public:
	static IWindow* construct(WindowAPI::T api);
};