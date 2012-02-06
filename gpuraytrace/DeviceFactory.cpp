#include "Common.h"
#include "DeviceFactory.h"

#include "DeviceDirect3D.h"

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
		if(!device->create())
		{
			delete device;
			device = nullptr;
		}
	}

	return device;
}