#pragma once

#include "IDevice.h"
#include "Factory.h"

class DeviceFactory : private Factory
{
public:
	static IDevice* construct(DeviceAPI::T api, IWindow* window);
};