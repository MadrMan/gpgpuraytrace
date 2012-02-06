#pragma once

#include "IWindow.h"
#include "Factory.h"

class WindowFactory : private Factory
{
public:
	static IWindow* construct(WindowAPI::T api);
};