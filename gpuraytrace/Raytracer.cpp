#include <Common.h>
#include "Raytracer.h"

#include "./Factories/DeviceFactory.h"
#include "./Factories/WindowFactory.h"
#include "./Factories/ICompute.h"
#include "./Factories/ITexture.h"
#include "./Graphics/Camera.h"
#include "./Graphics/IShaderVariable.h"

#include "IInput.h"
#include "Directory.h"
#include "Timer.h"

Raytracer::Raytracer()
{
	device = nullptr;
	window = nullptr;
	compute = nullptr;
	camera = nullptr;

	varView = nullptr;
	varProjection = nullptr;
	varEye = nullptr;

	texNoise = nullptr;
}

Raytracer::~Raytracer()
{
	delete compute;
	delete device;
	delete window;
	delete texNoise;
}

void Raytracer::run()
{
	WindowSettings ws;
	ws.width = 800;
	ws.height = 600;
	ws.fullscreen = false;

	//Create window
	window = WindowFactory::construct(WindowAPI::WinAPI, ws);
	if(!window) return;

	//Create device
	device = DeviceFactory::construct(DeviceAPI::Direct3D, window);
	if(!device) return;

	//Show window after device creation
	window->show();

	//Load textures
	texNoise = device->createTexture();
	texNoise->create("Media/noise1_small.png");

	//Create a new compute shader instance
	compute = device->createCompute();
	loadComputeShader();
	updateComputeVars();

	//Watch shader directory for changes
	Directory::get()->watch("shader", this, &Raytracer::loadComputeShader);

	//Register the escape key for exiting
	IInputAction* escape = window->getInput()->createAction();
	escape->registerKeyboard(VK_ESCAPE, 1.0f);

	//Create camera
	camera = new Camera();
	camera->setWindow(window);

	Timer* timer = Timer::get();
	timer->update(); timer->update();

	//Run while not exiting
	Logger() << "Running";

	float frameTime = 0.0f;
	int frames = 0;
	while(escape->getState() < 0.5f)
	{
		camera->update();
		updateCompute();

		//And present on screen
		device->present();

		//Update input and check if window still open
		if(window->update()) break;

		timer->update();

		frameTime += timer->getConstant();
		frames++;
		if(frameTime > 1.0f)
		{
			Logger() << "FPS: " << frames / frameTime;

			frameTime = fmod(frameTime, 1.0f);
			frames = 0;
		}

		//Logger() << "Y: " << XMVectorGetY(camera->position);
	}

	window->getInput()->destroyAction(escape);

	//Cleanup
	delete camera;

	Logger() << "Raytracer exit";
}

void Raytracer::updateCompute()
{
	if(compute->swap()) updateComputeVars();

	//Set variables
	if(varView && varProjection && varEye)
	{
		XMVECTOR determinant;
		XMMATRIX invTransView = XMMatrixTranspose(XMMatrixInverse(&determinant, camera->matView));
		varView->write(&invTransView);
		XMMATRIX transProjection = XMMatrixTranspose(camera->matProjection);
		varProjection->write(&transProjection);
		varEye->write(&camera->position);
	}

	//Run shader
	compute->run();
}

void Raytracer::updateComputeVars()
{
	varView = compute->getVariable("ViewInverse");
	varProjection = compute->getVariable("Projection");
	varEye = compute->getVariable("Eye");

	//Set resolution in the shader
	IShaderVariable* varScreenSize = compute->getVariable("ScreenSize");
	if(varScreenSize)
	{
		float screenSize[2] = { (float)window->getWindowSettings().width, (float)window->getWindowSettings().height };
		varScreenSize->write(screenSize);
	}

	compute->setTexture(0, texNoise);
}

void Raytracer::loadComputeShader()
{
	if(!compute->create("shader", "csmain.hlsl", "CSMain"))
	{
		Logger() << "Could not create shader";
	}
}