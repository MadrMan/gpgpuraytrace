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
#include "./Gameplay/Flyby.h"

#include "./Common/IInput.h"
#include "./Common/Directory.h"
#include "./Common/Timer.h"

Raytracer::Raytracer()
{
	device = nullptr;
	window = nullptr;
	compute = nullptr;
	camera = nullptr;
	flyby = nullptr;

	varView = nullptr;
	varEye = nullptr;
	varFrameData = nullptr;
	varMinDistance = nullptr;
	varMaxDistance = nullptr;
	varTime = nullptr;

	varCamView = nullptr;
	varCamEye = nullptr;
	varCamMinDistance = nullptr;
	varCamMaxDistance = nullptr;
	varCamResults = nullptr;
	varSunDirection = nullptr;

	texNoise1D = nullptr;
	texNoise2D = nullptr;

	timeOfDay = 0.3f; //6:30AM
	timeOfYear = 0.0f;
}

Raytracer::~Raytracer()
{
	delete compute;
	delete device;
	delete window;
	delete texNoise1D;
	delete texNoise2D;
}

void Raytracer::run()
{
	WindowSettings ws;
	//ws.width = 800;
	//ws.height = 600;

	const bool HD_RECORD_MODE = true;
	bool FIXED_FRAME_RATE = HD_RECORD_MODE;
	static const int TARGET_FRAME_RATE = 30;

	if(HD_RECORD_MODE)
	{
		ws.width = 1920;
		ws.height = 1080;
		ws.fullscreen = true;
	} else {
		ws.width = 1920 / 8;
		ws.height = 1080 / 8;
		ws.fullscreen = false;
	}

	//Create window
	window = WindowFactory::construct(WindowAPI::WinAPI, ws);
	if(!window) return;

	//Create device
	device = DeviceFactory::construct(DeviceAPI::Direct3D, window);
	if(!device) return;

	//Show window after device creation
	window->show();

	//Load textures
	noise = new Noise();
	noise->generate();

	texNoise2D = device->createTexture();
	texNoise2D->create(TextureDimensions::Texture2D, TextureFormat::R8G8B8A8_UINT, Noise::TEXTURE_SIZE, Noise::TEXTURE_SIZE, noise->permutations2D, TextureBinding::Texture, CPUAccess::None);
	//texNoise1D = device->createTexture();
	//texNoise1D->create(TextureDimensions::Texture1D, TextureFormat::R8G8B8A8_SNORM, Noise::TEXTURE_SIZE, 0, noise->permutations1D);
	//texNoise1D->create("Media/noise1_small.png");

	//Create camera
	camera = new Camera();
	camera->setWindow(window);

	//Create a new compute shader instance
	compute = device->createCompute();
	cameraCompute = device->createCompute();
	loadComputeShader();
	updateComputeVars();

	//Watch shader directory for changes
	Directory::get()->watch("shader", this, &Raytracer::loadComputeShader);

	//Register the escape key for exiting
	IInputAction* escape = window->getInput()->createAction();
	escape->registerKeyboard(VK_ESCAPE, 1.0f);

	IInputAction* toggleFlyby = window->getInput()->createAction();
	toggleFlyby->registerKeyboard(VK_F1, 1.0f);

	IInputAction* toggleRecording = window->getInput()->createAction();
	toggleRecording->registerKeyboard(VK_F2, 1.0f);

	//Flyby mode
	flyby = new Flyby(camera);

	Timer* timer = Timer::get();
	timer->update(); timer->update();

	IRecorder* recorder = RecorderFactory::construct(device, TARGET_FRAME_RATE);

	//Run while not exiting
	Logger() << "Running";

	float frameTime = 0.0f;
	int frames = 0;
	bool isFlybyMode = false;

	timer->update(); //Update for loading time

	if(HD_RECORD_MODE) 
	{
		if(recorder) recorder->start();
		flyby->reset();
		isFlybyMode = true;
	}

	while(escape->getState() < 0.5f)
	{
		timer->update();

		float thisFrameTime = FIXED_FRAME_RATE ?  1.0f / TARGET_FRAME_RATE : timer->getConstant();

		if(!FIXED_FRAME_RATE)
		{
			frameTime += thisFrameTime;
			frames++;
			if(frameTime > 1.0f)
			{
				Logger() << "FPS: " << frames / frameTime;

				frameTime = fmod(frameTime, 1.0f);
				frames = 0;
			}
		}

		if(toggleFlyby->isTriggered()) 
		{
			isFlybyMode = !isFlybyMode;
			flyby->reset();
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

		if(isFlybyMode)
		{
			flyby->fly(thisFrameTime);
		} else {
			camera->rotate();
			camera->move();
		}

		camera->update();
		updateCompute(thisFrameTime);

		//And present on screen
		device->present();

		//Update input and check if window still open
		if(window->update()) break;
	}

	if(recorder && recorder->isRecording()) recorder->stop();

	window->getInput()->destroyAction(escape);
	window->getInput()->destroyAction(toggleFlyby);
	window->getInput()->destroyAction(toggleRecording);

	//Cleanup
	delete camera;

	Logger() << "Raytracer exit";
}

struct SBFrameData
{
	float minDistance;
	float maxDistance;
};

void Raytracer::updateTerrain(float time)
{
	//Scale 'time' to a proper time value
	const float secondsInDay = 300.0f;
	timeOfDay += time / secondsInDay;

	//Fetch some data from the frame and calculate new constants
	if(varFrameData)
	{
		SBFrameData* fd = reinterpret_cast<SBFrameData*>(varFrameData->map());
		if(!fd) return;
		//Logger() << "Distance min: " << fd->minDistance << " max: " << fd->maxDistance;

		float minDist = std::max(0.05f, fd->minDistance * 0.9f);
		float maxDist = std::min(std::max(40.0f, fd->maxDistance * 1.2f), 8000.0f);

		const float MIN_DEFAULT = 20.0f;
		const float MAX_DEFAULT = 2000.0f;
		const float MINIMAL_DIFFERENCE = 10.0f;
		float difference = maxDist - minDist;
		if(difference < 0.0f)
		{
			minDist = MIN_DEFAULT;
			maxDist = MAX_DEFAULT;
		} else if(difference < MINIMAL_DIFFERENCE) {
			maxDist = minDist + MINIMAL_DIFFERENCE;
		}

		//Assign default inverse values (any large/small number would do)
		fd->minDistance = MAX_DEFAULT; //Swapped
		fd->maxDistance = MIN_DEFAULT; //Swapped

		varFrameData->unmap();

		//Ignore minimal range differences to prevent 'noise'
		float minDifference = minDist - curMinDistance;
		if(abs(minDifference) < minDist * 0.02f) minDist = curMinDistance;
		float maxDifference = maxDist - curMaxDistance;
		if(abs(maxDifference) < maxDist * 0.02f) maxDist = curMaxDistance;

		if(varMinDistance && varMaxDistance)
		{
			varMinDistance->write(&minDist);
			varMaxDistance->write(&maxDist);
		}

		if(varCamMinDistance && varCamMaxDistance)
		{
			varCamMinDistance->write(&minDist);
			varCamMaxDistance->write(&maxDist);
		}

		curMinDistance = minDist;
		curMaxDistance = maxDist;
	}

	if(varCamResults)
	{
		float* fd = reinterpret_cast<float*>(varCamResults->map());
		if(!fd) return;

		//char buf[32];
		//std::string res;
		//res.reserve(1000);
		//system("cls");
		/*for(int y = 0; y < CAMERA_DIST_RES; ++y)
		{
			for(int x = 0; x < CAMERA_DIST_RES; ++x)
			{
				sprintf(buf, "%5u", (unsigned int)fd[y * CAMERA_DIST_RES + x]);
				res += buf;
			}
			res += '\n';
		}*/

		memcpy(flyby->getCameraView().data(), fd, CAMERA_VIEW_RES * CAMERA_VIEW_RES * sizeof(CameraVision));

		varCamResults->unmap();
	}
}

void Raytracer::updateCompute(float time)
{
	updateComputeVars();

	updateTerrain(time);

	XMVECTOR determinant;
	XMMATRIX invTransView = XMMatrixTranspose(XMMatrixInverse(&determinant, camera->matView));

	//Set variables
	if(varView) varView->write(&invTransView);
	if(varEye) varEye->write(&camera->position);
	if(varCamView) varCamView->write(&invTransView);
	if(varCamEye) varCamEye->write(&camera->position);

	if(varTime)
	{
		float time = Timer::get()->getTime();
		varTime->write(&time);
	}

	if(varSunDirection)
	{
		//XMVECTOR sunDirection = XMVectorSet(0.6f, 0.6f, 0.6f, 0.0f); //
		//XMVECTOR sunDirection = XMVectorSet(0.0f, -0.1f, 0.91f, 0.0f); //sunset

		float sunY = -cosf(timeOfDay * XM_2PI);
		float sunX = -sinf(timeOfDay * XM_2PI);

		XMVECTOR sunDirection = XMVectorSet(sunX, sunY, 0.1f, 0.0f); //noon

		sunDirection = XMVector3Normalize(sunDirection);
		varSunDirection->write(&sunDirection);
	}

	//Run shader
	compute->run(device->getWindow()->getWindowSettings().width / 16, device->getWindow()->getWindowSettings().height / 16, 1);
	cameraCompute->run(2, 2, 1);
}

void Raytracer::updateComputeVars()
{
	XMMATRIX transProjection = XMMatrixTranspose(camera->matProjection);

	//Set resolution in the shader
	float screenSize[2] = { (float)window->getWindowSettings().width, (float)window->getWindowSettings().height };

	if(compute->swap())
	{
		varView = compute->getVariable("ViewInverse");
		varEye = compute->getVariable("Eye");
		varMinDistance = compute->getVariable("StartDistance");
		varMaxDistance = compute->getVariable("EndDistance");
		varTime = compute->getVariable("Time");
		varSunDirection = compute->getVariable("SunDirection");

		varFrameData = compute->getArray("FrameData");
		if(varFrameData)
		{
			varFrameData->create(true, 1);
		}

		IShaderVariable* varProjection = compute->getVariable("Projection");
		if(varProjection) varProjection->write(&transProjection);
		IShaderVariable* varScreenSize = compute->getVariable("ScreenSize");
		if(varScreenSize) varScreenSize->write(screenSize);
		IShaderVariable* varNoiseGrads = compute->getVariable("permGradients");
		if(varNoiseGrads) varNoiseGrads->write(noise->permutations1D);

		compute->setTexture(0, texNoise2D);
	}

	if(cameraCompute->swap())
	{
		varCamView = cameraCompute->getVariable("ViewInverse");
		varCamEye = cameraCompute->getVariable("Eye");
		varCamMinDistance = cameraCompute->getVariable("StartDistance");
		varCamMaxDistance = cameraCompute->getVariable("EndDistance");

		varCamResults = cameraCompute->getArray("CameraResults");
		if(varCamResults) 
		{
			varCamResults->create(false, CAMERA_VIEW_RES * CAMERA_VIEW_RES);
		}

		IShaderVariable* varCamProjection = cameraCompute->getVariable("Projection");
		if(varCamProjection) varCamProjection->write(&transProjection);
		IShaderVariable* varCamScreenSize = cameraCompute->getVariable("ScreenSize");
		if(varCamScreenSize) varCamScreenSize->write(screenSize);
		IShaderVariable* varCamNoiseGrads = cameraCompute->getVariable("permGradients");
		if(varCamNoiseGrads) varCamNoiseGrads->write(noise->permutations1D);

		cameraCompute->setTexture(0, texNoise2D);
	}
}

void Raytracer::loadComputeShader()
{
	if(!compute->create("shader", "tracescreen.hlsl", "CSMain"))
		Logger() << "Could not create screen shader";
	if(!cameraCompute->create("shader", "camerarays.hlsl", "CSMain"))
		Logger() << "Could not create camera shader";
}