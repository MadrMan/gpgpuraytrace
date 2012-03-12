#include <Common.h>
#include "Raytracer.h"

#include "./Factories/DeviceFactory.h"
#include "./Factories/WindowFactory.h"
#include "./Factories/ICompute.h"
#include "./Factories/ITexture.h"
#include "./Graphics/Camera.h"
#include "./Graphics/IShaderVariable.h"
#include "./Graphics/Noise.h"

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
	varFrameData = nullptr;
	varMinDistance = nullptr;
	varMaxDistance = nullptr;
	varTime = nullptr;

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

struct SBFrameData
{
	float minDistance;
	float maxDistance;
};

void Raytracer::updateTerrain()
{

	//Fetch some data from the frame and calculate new constants
	if(varFrameData && varMinDistance && varMaxDistance)
	{
		SBFrameData* fd = reinterpret_cast<SBFrameData*>(varFrameData->map());
		if(!fd) return;
		//Logger() << "Distance min: " << fd->minDistance << " max: " << fd->maxDistance;

		float minDist = std::max(0.05f, fd->minDistance * 0.9f);
		float maxDist = std::min(std::max(40.0f, fd->maxDistance * 1.1f), 10000.0f);

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

		varMinDistance->write(&minDist);
		varMaxDistance->write(&maxDist);

		//Assign default inverse values (any large/small number would do)
		fd->minDistance = MAX_DEFAULT; //Swapped
		fd->maxDistance = MIN_DEFAULT; //Swapped

		varFrameData->unmap();
	}
}

void Raytracer::updateCompute()
{
	if(compute->swap()) updateComputeVars();

	updateTerrain();

	//Set variables
	if(varView && varEye)
	{
		XMVECTOR determinant;
		XMMATRIX invTransView = XMMatrixTranspose(XMMatrixInverse(&determinant, camera->matView));
		varView->write(&invTransView);
		varEye->write(&camera->position);
	}

	if(varTime)
	{
		float time = Timer::get()->getTime();
		varTime->write(&time);
	}

	//Run shader
	compute->run();
}

void Raytracer::updateComputeVars()
{
	varView = compute->getVariable("ViewInverse");
	varEye = compute->getVariable("Eye");
	varFrameData = compute->getVariable("FrameData");
	varMinDistance = compute->getVariable("StartDistance");
	varMaxDistance = compute->getVariable("EndDistance");
	varTime = compute->getVariable("Time");

	varProjection = compute->getVariable("Projection");
	if(varProjection)
	{
		XMMATRIX transProjection = XMMatrixTranspose(camera->matProjection);
		varProjection->write(&transProjection);
	}

	//Set resolution in the shader
	IShaderVariable* varScreenSize = compute->getVariable("ScreenSize");
	if(varScreenSize)
	{
		float screenSize[2] = { (float)window->getWindowSettings().width, (float)window->getWindowSettings().height };
		varScreenSize->write(screenSize);
	}

	//Set other variables
	IShaderVariable* varSunDirection = compute->getVariable("SunDirection");
	if(varSunDirection)
	{
		XMVECTOR sunDirection = XMVectorSet(0.6f, 0.6f, 0.6f, 0.0f);
		sunDirection = XMVector3Normalize(sunDirection);
		varSunDirection->write(&sunDirection);
	}

	IShaderVariable* varNoiseGrads = compute->getVariable("permGradients");
	if(varNoiseGrads)
	{
		varNoiseGrads->write(noise->permutations1D);
	}

	compute->setTexture(0, texNoise2D);
	//compute->setTexture(1, texNoise1D);
}

void Raytracer::loadComputeShader()
{
	if(!compute->create("shader", "csmain.hlsl", "CSMain"))
	{
		Logger() << "Could not create shader";
	}
}