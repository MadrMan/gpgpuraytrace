#include "Common.h"
#include "Raytracer.h"

#include "DeviceFactory.h"
#include "WindowFactory.h"

Raytracer::Raytracer()
{
	device = nullptr;
	window = nullptr;
}

Raytracer::~Raytracer()
{
	delete device;
	delete window;
}

void Raytracer::run()
{
	WindowSettings ws;
	ws.width = 800;
	ws.height = 600;

	window = WindowFactory::construct(WindowAPI::WinAPI, ws);
	if(!window) return;

	device = DeviceFactory::construct(DeviceAPI::Direct3D, window);
	if(!device) return;

	window->show();

	for(;;)
	{
		device->present();

		if(window->update()) break;
	}

	Logger() << "Raytracer exit";
}