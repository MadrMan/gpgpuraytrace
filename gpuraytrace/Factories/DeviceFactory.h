#pragma once

#include "IDevice.h"
#include "Factory.h"

//! Factory used to create new devices
class DeviceFactory : private Factory
{
public:
	//! Construct a new device
	//! \param api API to create the new device with
	//! \param window Window to show the new device on
	//! \return nullptr if there was an error, otherwise a valid device
	static IDevice* construct(DeviceAPI::T api, IWindow* window);
};