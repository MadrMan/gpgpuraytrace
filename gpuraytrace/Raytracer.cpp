#include <Common.h>
#include "Raytracer.h"

#include "./Factories/DeviceFactory.h"
#include "./Factories/WindowFactory.h"
#include "./Factories/ICompute.h"
#include "./Factories/ITexture.h"
#include "./Graphics/Camera.h"
#include "./Graphics/IShaderVariable.h"
#include "./Graphics/Noise.h"
#include "./Gameplay/Flyby.h"

#include "IInput.h"
#include "Directory.h"
#include "Timer.h"

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

	texNoise1D = nullptr;
	texNoise2D = nullptr;
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
	ws.width = 1920 / 4;
	ws.height = 1080 / 4;
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
	noise = new Noise();
	noise->generate();

	texNoise2D = device->createTexture();
	texNoise2D->create(TextureDimensions::Texture2D, Noise::TEXTURE_SIZE, Noise::TEXTURE_SIZE, noise->permutations2D);
	//texNoise1D = device->createTexture();
	//texNoise1D->create(TextureDimensions::Texture1D, Noise::TEXTURE_SIZE, 0, noise->permutations1D);
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

	//Flyby mode
	flyby = new Flyby(camera);

	Timer* timer = Timer::get();
	timer->update(); timer->update();

	//Run while not exiting
	Logger() << "Running";

	float frameTime = 0.0f;
	int frames = 0;
	bool isFlybyMode = false;
	while(escape->getState() < 0.5f)
	{
		if(toggleFlyby->isTriggered()) 
		{
			isFlybyMode = !isFlybyMode;
			flyby->reset();
		}
		if(isFlybyMode)
		{
			flyby->fly();
		} else {
			camera->rotate();
			camera->move();
		}

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

struct SBFrameData
{
	float minDistance;
	float maxDistance;
};

void Raytracer::updateTerrain()
{
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

void Raytracer::updateCompute()
{
	bool result = compute->swap();
	result |= cameraCompute->swap();
	if(result) updateComputeVars();

	updateTerrain();

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

	//Run shader
	compute->run(device->getWindow()->getWindowSettings().width / 16, device->getWindow()->getWindowSettings().height / 16, 1);
	cameraCompute->run(1, 1, 1);
}

void Raytracer::updateComputeVars()
{
	varView = compute->getVariable("ViewInverse");
	varEye = compute->getVariable("Eye");
	varMinDistance = compute->getVariable("StartDistance");
	varMaxDistance = compute->getVariable("EndDistance");
	varTime = compute->getVariable("Time");

	varCamView = cameraCompute->getVariable("ViewInverse");
	varCamEye = cameraCompute->getVariable("Eye");
	varCamMinDistance = cameraCompute->getVariable("StartDistance");
	varCamMaxDistance = cameraCompute->getVariable("EndDistance");
	
	varFrameData = compute->getArray("FrameData");
	if(varFrameData)
	{
		varFrameData->create(true, 1);
	}

	varCamResults = cameraCompute->getArray("CameraResults");
	if(varCamResults) 
	{
		varCamResults->create(false, CAMERA_VIEW_RES * CAMERA_VIEW_RES);
	}

	IShaderVariable* varProjection = compute->getVariable("Projection");
	IShaderVariable* varCamProjection = cameraCompute->getVariable("Projection");
	XMMATRIX transProjection = XMMatrixTranspose(camera->matProjection);
	if(varProjection) varProjection->write(&transProjection);
	if(varCamProjection) varCamProjection->write(&transProjection);

	//Set resolution in the shader
	IShaderVariable* varScreenSize = compute->getVariable("ScreenSize");
	IShaderVariable* varCamScreenSize = cameraCompute->getVariable("ScreenSize");
	float screenSize[2] = { (float)window->getWindowSettings().width, (float)window->getWindowSettings().height };
	if(varScreenSize) varScreenSize->write(screenSize);
	if(varCamScreenSize) varCamScreenSize->write(screenSize);

	//Set other variables
	IShaderVariable* varSunDirection = compute->getVariable("SunDirection");
	if(varSunDirection)
	{
		XMVECTOR sunDirection = XMVectorSet(0.6f, 0.6f, 0.6f, 0.0f);
		sunDirection = XMVector3Normalize(sunDirection);
		varSunDirection->write(&sunDirection);
	}

	IShaderVariable* varNoiseGrads = compute->getVariable("permGradients");
	IShaderVariable* varCamNoiseGrads = cameraCompute->getVariable("permGradients");
	if(varNoiseGrads) varNoiseGrads->write(noise->permutations1D);
	if(varCamNoiseGrads) varCamNoiseGrads->write(noise->permutations1D);

	compute->setTexture(0, texNoise2D);
	cameraCompute->setTexture(0, texNoise2D);
	//compute->setTexture(1, texNoise1D);
}

void Raytracer::loadComputeShader()
{
	if(!compute->create("shader", "tracescreen.hlsl", "CSMain"))
		Logger() << "Could not create screen shader";
	if(!cameraCompute->create("shader", "camerarays.hlsl", "CSMain"))
		Logger() << "Could not create camera shader";
}