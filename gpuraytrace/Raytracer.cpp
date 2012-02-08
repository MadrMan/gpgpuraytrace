#include "Common.h"
#include "Raytracer.h"

#include "DeviceFactory.h"
#include "WindowFactory.h"
#include "ICompute.h"

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

	ICompute* compute = device->createCompute();
	if(!compute->create("shader/CSMain.hlsl", "CSMain"))
	{
		Logger() << "Could not create shader";

		delete compute;
		return;
	}

	for(;;)
	{
		device->present();

		if(window->update()) break;
	}

	Logger() << "Raytracer exit";
}