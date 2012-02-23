#include <Common.h>
#include "Raytracer.h"

#include "./Factories/DeviceFactory.h"
#include "./Factories/WindowFactory.h"
#include "./Factories/ICompute.h"
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
}

Raytracer::~Raytracer()
{
	delete compute;
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
	IInputAction* escape = window->getInput()->createAction();
	escape->registerKeyboard(VK_ESCAPE, 1.0f);

	//Create camera
	Camera* camera = new Camera();
	camera->setWindow(window);

	//Register camera input
	IInputAction* moveSide = window->getInput()->createAction();
	IInputAction* moveForward = window->getInput()->createAction();
	IInputAction* rotateLR = window->getInput()->createAction();
	IInputAction* rotateUD = window->getInput()->createAction();
	moveSide->registerKeyboard('A', -1.0f, TriggerType::OnHold);
	moveSide->registerKeyboard('D', 1.0f, TriggerType::OnHold);
	moveForward->registerKeyboard('W', 1.0f, TriggerType::OnHold);
	moveForward->registerKeyboard('S', -1.0f, TriggerType::OnHold);
	rotateLR->registerMouseAxis(0);
	rotateUD->registerMouseAxis(1);

	float cameraRotation[3] = {0};
	IShaderVariable* varView = compute->getVariable("ViewInverse");
	IShaderVariable* varProjection = compute->getVariable("Projection");

	Timer* timer = Timer::get();
	timer->update(); timer->update();

	//Run while not exiting
	Logger() << "Running";

	float frameTime = 0.0f;
	int frames = 0;
	while(escape->getState() < 0.5f)
	{
		//Rotate camera
		cameraRotation[0] += rotateLR->getState() * 0.01f;
		cameraRotation[1] += rotateUD->getState() * 0.01f;
		camera->rotation = XMQuaternionRotationRollPitchYaw(cameraRotation[0], cameraRotation[1], cameraRotation[2]);

		//Move camera
		XMVECTOR front = XMVector3Rotate(XM_FRONT, camera->rotation);
	    camera->position = XMVectorAdd(camera->position, front * moveForward->getState());
		XMVECTOR right = XMVector3Rotate(front, XMQuaternionRotationRollPitchYaw(0.0f, XM_PIDIV2, 0.0f));
		camera->position = XMVectorAdd(camera->position, right * moveSide->getState());

		//Update
		camera->update();

		//Set variables
		if(varView && varProjection)
		{
			XMVECTOR determinant;
			XMMATRIX invView = XMMatrixInverse(&determinant, camera->matView);
			varView->write(&invView);
			varProjection->write(&camera->matProjection);
		}

		//Run shader
		compute->run();

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
			//Logger() << "FPS: " << timer->getTime();

			frameTime = fmod(frameTime, 1.0f);
			frames = 0;
		}
	}

	//Cleanup
	delete camera;
	window->getInput()->destroyAction(moveSide);
	window->getInput()->destroyAction(moveForward);
	window->getInput()->destroyAction(escape);

	Logger() << "Raytracer exit";
}

void Raytracer::loadComputeShader()
{
	if(!compute->create("shader/CSMain.hlsl", "CSMain"))
	{
		Logger() << "Could not create shader";
	}


}