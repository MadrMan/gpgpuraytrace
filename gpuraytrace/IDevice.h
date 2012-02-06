#pragma once

#include "IWindow.h"

namespace DeviceAPI { enum T
{
	Direct3D,
	OpenGL
};}

class IDevice
{
public:
	virtual ~IDevice() { }

	virtual bool create() = 0;
	virtual void present() = 0;

	DeviceAPI::T getAPI() const { return api; }
	IWindow* getWindow() const { return window; }

protected:
	IDevice(DeviceAPI::T api, IWindow* window) : api(api), window(window) { }
	

private:
	const DeviceAPI::T api;
	IWindow* const window;
};

