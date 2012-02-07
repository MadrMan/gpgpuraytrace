#pragma once

#include "IWindow.h"

namespace DeviceAPI { enum T
{
	None,
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
	IDevice(IDevice const&) : api(DeviceAPI::None), window(nullptr) {};         
    IDevice& operator=(IDevice const&){};  

	const DeviceAPI::T api;
	IWindow* const window;
};

