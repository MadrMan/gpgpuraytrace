#pragma once

#include "IDevice.h"

class DeviceDirect3D : public IDevice
{
public:
	DeviceDirect3D(IWindow* window);
	virtual ~DeviceDirect3D();

	virtual bool create() override;
	virtual void present() override;

private:

};

