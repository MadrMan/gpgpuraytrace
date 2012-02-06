#pragma once

#include "IDevice.h"

class DeviceFactory
{
public:
	static IDevice* construct(DeviceAPI::T api, IWindow* window);
};