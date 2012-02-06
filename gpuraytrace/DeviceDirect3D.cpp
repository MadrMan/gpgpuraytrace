#include "Common.h"
#include "DeviceDirect3D.h"

DeviceDirect3D::DeviceDirect3D(IWindow* window) : IDevice(DeviceAPI::Direct3D, window)
{

}

DeviceDirect3D::~DeviceDirect3D()
{

}

bool DeviceDirect3D::create()
{
	return false;
}

void DeviceDirect3D::present()
{

}