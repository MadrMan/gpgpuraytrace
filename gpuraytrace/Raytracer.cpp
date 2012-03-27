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
#include "./Common/Settings.h"

Raytracer::Raytracer()
{
	device = nullptr;
	window = nullptr;
	compute = nullptr;
	camera = nullptr;
	flyby = nullptr;

	varView = nullptr;
	varEye = nullptr;
	varCamFrameData = nullptr;
	varMinDistance = nullptr;
	varMaxDistance = nullptr;
	varTime = nullptr;
	varThreadOffset = nullptr;
	varSunDirection = nullptr;

	varCamView = nullptr;
	varCamEye = nullptr;
	varCamMinDistance = nullptr;
	varCamMaxDistance = nullptr;
	varCamResults = nullptr;
	
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
	static const int TARGET_FRAME_RATE = 25;
	Mode mode = MODE_TEST;

	//Create window
	window = WindowFactory::construct(WindowAPI::WinAPI, mode.ws);
	if(!window) return;

	//Create device
	device = DeviceFactory::construct(DeviceAPI::Direct3D, window);
	if(!device) return;

	//Show window after device creation
	window->show();

	//Get tile sizes for the rendering
	calculateTileSizes();

	//Load textures
	noise = new Noise();
	noise->generate(mode.randomLandscape);

	texNoise2D = device->createTexture();
	texNoise2D->create(TextureDimensions::Texture2D, TextureFormat::R8G8B8A8_UINT, Noise::TEXTURE_SIZE, Noise::TEXTURE_SIZE, noise->permutations2D, TextureBinding::Texture, CPUAccess::None);
	//texNoise1D = device->createTexture();
	//texNoise1D->create(TextureDimensions::Texture1D, TextureFormat::R8G8B8A8_SNORM, Noise::TEXTURE_SIZE, 0, noise->permutations1D);
	//texNoise1D->create("Media/noise1_small.png");

	//1 = smudgey brown forest floor
	//2 = brown mossy rock
	//3 = gray/lumpy repetative rock
	//4 = exploded mushroom brown
	//5 = brown mud metal
	//6 = mossy rock
	//7 = grass
	//8 = muddy forest floor
	//9 = reef-ish gray
	//10 = graygreenbrown rock
	//11 = brown rocksand
	//12 = organic brown/yellow things
	//13 = gray rock
	//14 = brown forest floor
	//15 = gray searock
	//16 = dirt with gray moss patches
	//17 = rock wall
	//18 = forest pebblefloor
	//19 = icey snowrock

	for(int x = 0; x < 6; x++)
		texDiffuse[x] = device->createTexture();
	texDiffuse[0]->create("Media/textures/lichen9.dds");
	texDiffuse[1]->create("Media/textures/lichen13.dds");
	texDiffuse[2]->create("Media/textures/lichen16.dds");
	texDiffuse[3]->create("Media/textures/lichen7.dds");
	//texDiffuse[4]->create("Media/textures/lichen10.dds");
	//texDiffuse[5]->create("Media/textures/lichen16.dds");

	/*texDiffuse[0]->create("Media/textures/lichen19.dds");
	texDiffuse[1]->create("Media/textures/lichen19.dds");
	texDiffuse[2]->create("Media/textures/lichen19.dds");
	texDiffuse[3]->create("Media/textures/lichen7.dds");
	texDiffuse[4]->create("Media/textures/lichen13.dds");
	texDiffuse[5]->create("Media/textures/lichen15.dds");*/

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

	if(mode.recordMode) 
	{
		if(recorder) recorder->start();
		flyby->reset();
		isFlybyMode = true;
	}

	while(escape->getState() < 0.5f)
	{
		timer->update();

		float thisFrameTime = mode.fixedFrameRate ?  1.0f / TARGET_FRAME_RATE : timer->getConstant();

		if(!mode.fixedFrameRate)
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

void Raytracer::calculateTileSizes()
{
	int resx = device->getWindow()->getWindowSettings().width;
	int resy = device->getWindow()->getWindowSettings().height;

	const int DEFAULT_THREADSIZE = 16;
	const int DEFAULT_TILEPIXELS = 512;

	//Calculate amount of tiles needed
	tilesX = (int)ceilf(resx / (float)DEFAULT_TILEPIXELS);
	tilesY = (int)ceilf(resy / (float)DEFAULT_TILEPIXELS);

	//Amount of pixels per tile
	tileSizeX = resx / tilesX;
	tileSizeY = resy / tilesY;

	//Calculate fitting thread sizes
	threadSizeX = DEFAULT_THREADSIZE;
	threadSizeY = DEFAULT_THREADSIZE;
	while(tileSizeX % threadSizeX) ++threadSizeX;
	while(tileSizeY % threadSizeY) ++threadSizeY;

	//These are the blocks to be dispatched per tile
	dispatchSizeX = tileSizeX / threadSizeX;
	dispatchSizeY = tileSizeY / threadSizeY;
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
	if(varCamFrameData)
	{
		SBFrameData* fd = reinterpret_cast<SBFrameData*>(varCamFrameData->map());
		if(!fd) return;
		//Logger() << "Distance min: " << fd->minDistance << " max: " << fd->maxDistance;

		//float minDist = std::max(0.05f, fd->minDistance * 0.9f);
		//float maxDist = std::min(std::max(40.0f, fd->maxDistance * 1.2f), 8000.0f);

		float minDist = fd->minDistance;
		float maxDist = fd->maxDistance;

		const float MIN_DEFAULT = 20.0f;
		const float MAX_DEFAULT = 2000.0f;
		const float MINIMAL_DIFFERENCE = 10.0f;
		/*float difference = maxDist - minDist;
		if(difference < 0.0f)
		{
			minDist = MIN_DEFAULT;
			maxDist = MAX_DEFAULT;
		} else if(difference < MINIMAL_DIFFERENCE) {
			maxDist = minDist + MINIMAL_DIFFERENCE;
		}*/

		//Assign default inverse values (any large/small number would do)
		fd->minDistance = MAX_DEFAULT; //Swapped
		fd->maxDistance = MIN_DEFAULT; //Swapped

		varCamFrameData->unmap();

		//Ignore minimal range differences to prevent 'noise'
		/*float minDifference = minDist - curMinDistance;
		if(abs(minDifference) < minDist * 0.02f) minDist = curMinDistance;
		float maxDifference = maxDist - curMaxDistance;
		if(abs(maxDifference) < maxDist * 0.02f) maxDist = curMaxDistance;*/

		//if(varMinDistance && varMaxDistance)
		//{
			varMinDistance->write(&minDist);
			varMaxDistance->write(&maxDist);
		//}

		//if(varCamMinDistance && varCamMaxDistance)
		//{
			//varCamMinDistance->write(&minDist);
			//varCamMaxDistance->write(&maxDist);
		//}
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

	runCompute();
}

void Raytracer::runCompute()
{
	//Run shaders
	//First, trace a downsampled version of the entire screen to determine distance and such
	cameraCompute->run(2, 2, 1);

	//Then, draw rest of the screen in a tiled manner
	if(varThreadOffset)
	{
		for(int x = 0; x < tilesX; ++x)
		{
			for(int y = 0; y < tilesY; ++y)
			{
				unsigned int offset[] = { x * dispatchSizeX * threadSizeX, y * dispatchSizeY * threadSizeY};
				varThreadOffset->write(offset);
				compute->run(dispatchSizeX, dispatchSizeY, 1);
				
				//device->present();
			}
		}
	} else {
		compute->run(dispatchSizeX * tilesX, dispatchSizeY * tilesY, 1);
	}
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
		varThreadOffset = compute->getVariable("ThreadOffset");

		IShaderVariable* varProjection = compute->getVariable("Projection");
		if(varProjection) varProjection->write(&transProjection);
		IShaderVariable* varScreenSize = compute->getVariable("ScreenSize");
		if(varScreenSize) varScreenSize->write(screenSize);
		IShaderVariable* varNoiseGrads = compute->getVariable("permGradients");
		if(varNoiseGrads) varNoiseGrads->write(noise->permutations1D);

		compute->setTexture(0, texNoise2D);
		for(int x = 0; x < 6; x++)
			compute->setTexture(x + 1, texDiffuse[x]);
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

		varCamFrameData = cameraCompute->getArray("FrameData");
		if(varCamFrameData)
		{
			varCamFrameData->create(true, 1);
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
	ThreadSize screenThreads = {threadSizeX, threadSizeY, 1};
	if(!compute->create("shader", "tracescreen.hlsl", "CSMain", screenThreads))
		Logger() << "Could not create screen shader";

	ThreadSize cameraThreads = {16, 16, 1};
	if(!cameraCompute->create("shader", "camerarays.hlsl", "CSMain", cameraThreads))
		Logger() << "Could not create camera shader";
}