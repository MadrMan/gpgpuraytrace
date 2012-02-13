#include "Common.h"
#include "Raytracer.h"

#include "DeviceFactory.h"
#include "WindowFactory.h"
#include "ICompute.h"
#include "IInput.h"
#include "Directory.h"

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

	//Create window
	window = WindowFactory::construct(WindowAPI::WinAPI, ws);
	if(!window) return;

	//Create device
	device = DeviceFactory::construct(DeviceAPI::Direct3D, window);
	if(!device) return;

	//Show window after device creation
	window->show();

	//Create a new compute shader instance
	compute = device->createCompute();
	loadComputeShader();

	//Watch shader directory for changes
	Directory::get()->watch("shader", this, &Raytracer::loadComputeShader);

	//Register the escape key for exiting
	IInputAction* action = window->getInput()->createAction();
	action->registerKeyboard(VK_ESCAPE, 1.0f);

	//Run while not exiting
	Logger() << "Running";
	while(action->getState() < 0.5f)
	{
		compute->run();
		device->present();

		if(window->update()) break;
	}

	//Cleanup
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