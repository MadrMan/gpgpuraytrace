#include <Common.h>
#include "DeviceFactory.h"

#include "../Adapters/DeviceDirect3D.h"

IDevice* DeviceFactory::construct(DeviceAPI::T api, IWindow* window)
{
	IDevice* device = nullptr;

	switch(api)
	{
	case DeviceAPI::Direct3D:
		device = new DeviceDirect3D(window);
		break;
	case DeviceAPI::OpenGL:
		break;
	default:
		break;
	}

	if(device)
	{
		create(&device);
	} else {
		Logger() << "Not a valid option for " << __FUNCTION__;
	}

	return device;
}