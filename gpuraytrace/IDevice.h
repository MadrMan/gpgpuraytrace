#pragma once

#include "IWindow.h"

namespace DeviceAPI { enum T
{
	None, //!< Invalid device type
	Direct3D, //!< Direct3D device type
	OpenGL //!< OpenGL device type
};}

//! Interfaces inherited by all devices
class IDevice
{
public:
	//! Destructor
	virtual ~IDevice() { }

	//! Create the window using the parameters supplied to the constructor
	//! \return True if successful, false if not
	virtual bool create() = 0;

	//! Present the current frame to the screen
	virtual void present() = 0;

	//! Get the API used to create the device with
	DeviceAPI::T getAPI() const { return api; }

	//! Get the window the device is set up on
	IWindow* getWindow() const { return window; }

protected:
	IDevice(DeviceAPI::T api, IWindow* window) : api(api), window(window) { }
	
private:
	IDevice(IDevice const&) : api(DeviceAPI::None), window(nullptr) {};         
    IDevice& operator=(IDevice const&){};  

	const DeviceAPI::T api;
	IWindow* const window;
};

