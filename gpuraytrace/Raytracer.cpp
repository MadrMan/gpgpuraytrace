#include "Common.h"
#include "Raytracer.h"

#include "DeviceFactory.h"
#include "WindowFactory.h"
#include "ICompute.h"
#include "IInput.h"

Raytracer::Raytracer()
{
	device = nullptr;
	window = nullptr;
	compute = nullptr;
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

	compute = device->createCompute();
	loadComputeShader();

	IInputAction* action = window->getInput()->createAction();
	action->registerKeyboard(VK_ESCAPE, 1.0f);

	Logger() << "Running";
	while(action->getState() < 0.5f)
	{
		compute->run();
		device->present();

		if(window->update()) break;
	}

	window->getInput()->destroyAction(action);
	Logger() << "Raytracer exit";
}

void Raytracer::loadComputeShader()
{
	if(!compute->create("shader/CSMain.hlsl", "CSMain"))
	{
		Logger() << "Could not create shader";
	}
}