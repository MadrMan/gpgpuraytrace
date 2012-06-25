#include <Common.h>
#include "Raytracer.h"

#include "./Factories/DeviceFactory.h"
#include "./Factories/WindowFactory.h"
#include "./Factories/RecorderFactory.h"
#include "./Factories/ICompute.h"
#include "./Factories/ITexture.h"

#include "./Graphics/Camera.h"
#include "./Graphics/IShaderVariable.h"
#include "./Graphics/Noise.h"
#include "./Graphics/Terrain.h"

#include "./Gameplay/Flyby.h"

#include "./Common/IInput.h"
#include "./Common/Directory.h"
#include "./Common/Timer.h"
#include "./Common/Settings.h"
#include "./Common/VFS.h"

Raytracer::Raytracer()
{
	device = nullptr;
	window = nullptr;
	camera = nullptr;
	flyby = nullptr;
	terrain = nullptr;

	timeOfDay = 0.3f; //6:30AM
	timeOfYear = 0.0f;

	VFS::get()->addPath("Media/common");
}

Raytracer::~Raytracer()
{
	delete terrain;
	delete device;
	delete window;
}

void Raytracer::run(const Mode& mode, const std::string landscape)
{
	static const int TARGET_FRAME_RATE = 25;

	//Create window
	window = WindowFactory::construct(WindowAPI::WinAPI, mode.ws);
	if(!window) return;

	//Create device
	device = DeviceFactory::construct(DeviceAPI::Direct3D, window);
	if(!device) return;

	//Show window after device creation
	window->show();

	//Create camera
	camera = new Camera();
	camera->setWindow(window);

	//Register the escape key for exiting
	IInputAction* escape = window->getInput()->createAction();
	escape->registerKeyboard(VK_ESCAPE, 1.0f);

	IInputAction* toggleFlyby = window->getInput()->createAction();
	toggleFlyby->registerKeyboard(VK_F1, 1.0f);

	//register key for recording
	IInputAction* toggleRecording = window->getInput()->createAction();
	toggleRecording->registerKeyboard(VK_F2, 1.0f);

	//register keys to change timeofday
	IInputAction* increaseTimeOfDay = window->getInput()->createAction();	
	increaseTimeOfDay->registerKeyboard(VK_ADD, 1.0f, TriggerType::OnHold);	
	increaseTimeOfDay->registerKeyboard(VK_OEM_PLUS, 1.0f, TriggerType::OnHold);	
	increaseTimeOfDay->registerKeyboard(VK_OEM_MINUS, -1.0f, TriggerType::OnHold);
	increaseTimeOfDay->registerKeyboard(VK_SUBTRACT, -1.0f, TriggerType::OnHold);
	increaseTimeOfDay->registerKeyboard(VK_OEM_MINUS, -1.0f, TriggerType::OnHold);

	//dump position
	IInputAction* dumpPos = window->getInput()->createAction();	
	dumpPos->registerKeyboard('L',1.0f);

	IInputAction* resetPos = window->getInput()->createAction();	
	resetPos->registerKeyboard('R',1.0f);

	//Create terrain
	terrain = new Terrain(device, landscape, mode);
	terrain->create();
	terrain->setCamera(camera);

	reloadTerrain();

	//Watch media directory for changes
	Directory::get()->watch("media", this, &Raytracer::reloadTerrain);

	flyby = new Flyby(camera);

	Timer* timer = Timer::get();
	timer->update(); timer->update();

	IRecorder* recorder = RecorderFactory::construct(device, TARGET_FRAME_RATE, mode.fixedFrameRate);

	//Run while not exiting
	Logger() << "Running";


	float frameTime = 0.0f;
	int frames = 0;
	bool isFlybyMode = false;

	timer->update(); //Update for loading time
	
	float lastTimeTrackingKinect = timer->getTime(); 
	const float timeOutKinect = 5.0f;

	if(mode.recordMode) 
	{
		if(recorder) recorder->start();
		flyby->reset();
		isFlybyMode = true;
	}
	bool kinectIsTracking = false;
	while(escape->getState() < 0.5f)
	{
		//reset position
		if(resetPos->isTriggered())
		{
			Logger() << "Camera position reset.";
			camera->resetPosition();
		}
		
		//position
		if(dumpPos->getState() == 1.0f)
		{
			Logger() << "Position :"  << XMVectorGetX(camera->position) << " " << XMVectorGetY(camera->position) << " " << XMVectorGetZ(camera->position);
			Logger() << "Rotation : " << camera->rotationEuler[0] << " " << camera->rotationEuler[1] << " " << camera->rotationEuler[2];
		}
		
		timer->update();
		float thisFrameTime = mode.fixedFrameRate ?  1.0f / TARGET_FRAME_RATE : timer->getConstant();
		
		if(mode.autoFlyby)
		{	
			bool isTracking = camera->isTracking();
			if(isTracking != kinectIsTracking)
			{
				kinectIsTracking = isTracking;
				
				if(kinectIsTracking) 
				{
					Logger() << "Kinect started tracking. Stopping flyby.";
				} else {
					Logger() << "Kinect stopped tracking. Starting flyby.";
				}
				isFlybyMode = !isTracking;
			}
		}

		if(!mode.fixedFrameRate)
		{
			frameTime += thisFrameTime;
			frames++;
			if(frameTime > 1.0f)
			{
				Logger() << "FPS: " << frames / frameTime;
				frameTime = 0.0f;
				frames = 0;
			}
		}
		
		if(toggleFlyby->isTriggered()) 
		{
			if(mode.autoFlyby)
			{
				Logger() << "Flyby mode disabled when tracking width kinect.";
			}else{
				isFlybyMode = !isFlybyMode;
				flyby->reset();
			}
		}

		if(recorder && toggleRecording->isTriggered())
		{
			if(recorder->isRecording())
			{
				recorder->stop();
				Logger() << "=== Finished recording ===";
			}
			else
			{
				Logger() << "=== Starting recording ===";
				recorder->start();
			}
		}

		//terrain->getCameraResults(flyby);
		if(isFlybyMode)
		{
			flyby->fly(thisFrameTime, terrain);
		} else {
			camera->rotate();
			camera->move();
		}

		timeOfDay += increaseTimeOfDay->getState() * (thisFrameTime/25.0f);		
		camera->update();

		updateCompute(thisFrameTime, mode);
		terrain->render();

		//And present on screen
		device->present();

		//Update input and check if window still open
		if(window->update()) break;
	}

	if(recorder && recorder->isRecording()) recorder->stop();

	window->getInput()->destroyAction(escape);
	window->getInput()->destroyAction(toggleFlyby);
	window->getInput()->destroyAction(toggleRecording);
	window->getInput()->destroyAction(dumpPos);
	window->getInput()->destroyAction(resetPos);

	//Cleanup
	delete camera;

	Logger() << "Raytracer exit";
}

void Raytracer::updateCompute(float time, const Mode& mode)
{
	//Scale 'time' to a proper time value
	const float secondsInDay = 300.0f;
	if(mode.incrementDayTime) timeOfDay += time / secondsInDay;

	terrain->updateTerrain(time);
	terrain->setTimeOfDay(timeOfDay);
}

void Raytracer::reloadTerrain()
{
	terrain->reload();
}