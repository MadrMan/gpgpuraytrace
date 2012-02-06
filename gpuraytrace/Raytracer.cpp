#include "Common.h"
#include "Raytracer.h"

#include "DeviceFactory.h"
#include "WindowFactory.h"

Raytracer::Raytracer()
{
	device = nullptr;
}

Raytracer::~Raytracer()
{
	delete device;
}

void Raytracer::run()
{
	IWindow* window = WindowFactory::construct(WindowAPI::WinAPI);
	if(!window) return;

	device = DeviceFactory::construct(DeviceAPI::Direct3D, window);
}