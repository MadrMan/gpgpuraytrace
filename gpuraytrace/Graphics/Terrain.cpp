#include <Common.h>
#include "Terrain.h"
#include "../Factories/IDevice.h"
#include "../Factories/ITexture.h"
#include "../Factories/ICompute.h"
#include "../Gameplay/Flyby.h"
#include "../Common/Logger.h"
#include "../Common/VFS.h"

#include "Camera.h"
#include "Noise.h"

struct SBFrameData
{
	float minDistance;
	float maxDistance;
};

const unsigned int CAMERA_VIEW_ELEMENTS = CAMERA_VIEW_RES * CAMERA_VIEW_RES;

Terrain::Terrain(IDevice* device, const std::string& theme, const Mode& mode) : device(device), theme(theme), mode(mode)
{
	VFS::get()->addPath("Media/" + theme);

	//memset(texDiffuse, 0, sizeof(texDiffuse));

	compute = nullptr;
	cameraCompute = nullptr;

	varView = nullptr;
	varEye = nullptr;
	varCamFrameData = nullptr;
	varMinDistance = nullptr;
	varMaxDistance = nullptr;
	varTime = nullptr;
	varThreadOffset = nullptr;
	varSunDirection = nullptr;
	varCellDistance = nullptr;

	varCamView = nullptr;
	varCamEye = nullptr;
	varCamMinDistance = nullptr;
	varCamMaxDistance = nullptr;
	varCamResults = nullptr;

	noise = nullptr;
	texNoise1D = nullptr;
	texNoise2D = nullptr;

	curFarDist = 0.0f;

	cameraView.resize(CAMERA_VIEW_RES * CAMERA_VIEW_RES);
}

Terrain::~Terrain()
{
	delete compute;
	delete cameraCompute;
	delete noise;
	delete texNoise1D;
	delete texNoise2D;
}

void Terrain::setCamera(Camera* camera)
{
	this->camera = camera;
}

void Terrain::create()
{
	calculateTileSizes();

	//Create a new compute shader instance
	compute = device->createCompute();
	cameraCompute = device->createCompute();

	//Noise
	noise = new Noise();
	noise->generate(mode.randomLandscape);

	texNoise2D = device->createTexture();
	texNoise2D->create(TextureDimensions::Texture2D, TextureFormat::R8G8B8A8_UINT, Noise::TEXTURE_SIZE, Noise::TEXTURE_SIZE, noise->permutations2D, TextureBinding::Texture, CPUAccess::None);
	//texNoise1D = device->createTexture();
	//texNoise1D->create(TextureDimensions::Texture1D, TextureFormat::R8G8B8A8_SNORM, Noise::TEXTURE_SIZE, 0, noise->permutations1D);
	//texNoise1D->create("Media/noise1_small.png");

	loadTextures();
	calculateTileSizes();
}

void Terrain::reload()
{
	std::vector<MacroType> macros;
	if(mode.recordMode) macros.push_back(MacroType("RECORDING", "1"));

	const static ThreadSize screenThreads = {threadSizeX, threadSizeY, 1};
	if(!compute->create("shaders", "tracescreen.hlsl", "CSMain", screenThreads, macros))
		Logger() << "Could not create screen shader";

	const static ThreadSize cameraThreads = {CAMERA_THREAD_RES, CAMERA_THREAD_RES, 1};
	if(!cameraCompute->create("shaders", "camerarays.hlsl", "CSMain", cameraThreads, macros))
		Logger() << "Could not create camera shader";
}

void Terrain::render()
{
	//Check if the shaders changed
	updateShaders();

	//Run shaders
	//First, trace a downsampled version of the entire screen to determine distance and such
	cameraCompute->run(
		(unsigned int)std::ceil(CAMERA_VIEW_RES / (float)CAMERA_THREAD_RES),
		(unsigned int)std::ceil(CAMERA_VIEW_RES / (float)CAMERA_THREAD_RES), 1);

	//Get results
	getCameraResults();

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
				
				device->flush();
			}
		}
	} else {
		compute->run(dispatchSizeX * tilesX, dispatchSizeY * tilesY, 1);
	}
}

void Terrain::updateShaders()
{
	XMMATRIX transProjection = XMMatrixTranspose(camera->matProjection);

	//Set resolution in the shader
	float screenSize[2] = { 
		(float)camera->getWindow()->getWindowSettings().width,
		(float)camera->getWindow()->getWindowSettings().height };

	if(compute->swap())
	{
		varView = compute->getVariable("ViewInverse");
		varEye = compute->getVariable("Eye");
		varMinDistance = compute->getVariable("StartDistance");
		varMaxDistance = compute->getVariable("EndDistance");
		varTime = compute->getVariable("Time");
		varSunDirection = compute->getVariable("SunDirection");
		varThreadOffset = compute->getVariable("ThreadOffset");

		varCellDistance = compute->getArray("CellDistance");
		if(varCellDistance)
		{
			varCellDistance->create(CAMERA_VIEW_RES * CAMERA_VIEW_RES);
		}

		IShaderVariable* varProjection = compute->getVariable("Projection");
		if(varProjection) varProjection->write(&transProjection);
		IShaderVariable* varScreenSize = compute->getVariable("ScreenSize");
		if(varScreenSize) varScreenSize->write(screenSize);
		IShaderVariable* varNoiseGrads = compute->getVariable("permGradients");
		if(varNoiseGrads) varNoiseGrads->write(noise->permutations1D);

		compute->setTexture(0, texNoise2D);

		/*for(int x = 0; x < 6; x++)
		{
			compute->setTexture(x + 1, texDiffuse[x]);
		}*/
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
			varCamResults->create(CAMERA_VIEW_RES * CAMERA_VIEW_RES);
		}

		varCamFrameData = cameraCompute->getArray("FrameData");
		if(varCamFrameData)
		{
			varCamFrameData->create(1);
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

void Terrain::calculateTileSizes()
{
	int resx = device->getWindow()->getWindowSettings().width;
	int resy = device->getWindow()->getWindowSettings().height;

	//Way smaller multiplier when recording cause it is so much slower
	const int divisor = mode.recordMode ? 4 : 1; 

	//Desired threads per tile
	const int DEFAULT_THREADSIZE = 16;

	//Default amount of pixels per tile
	const int DEFAULT_TILEPIXELS_X = 1024 / divisor;
	const int DEFAULT_TILEPIXELS_Y = 512 / divisor;

	//Calculate amount of tiles needed
	tilesX = (int)ceilf(resx / (float)DEFAULT_TILEPIXELS_X);
	tilesY = (int)ceilf(resy / (float)DEFAULT_TILEPIXELS_Y);

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

	Logger() << "Dispatch size = " << dispatchSizeX << ":" << dispatchSizeY << " Thread size = " << threadSizeX << ":" << threadSizeY << " Tiles = " << tilesX << ":" << tilesY;
}

void Terrain::loadTextures()
{
	//Load textures

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

	/*for(int x = 0; x < 4; x++)
		texDiffuse[x] = device->createTexture();
	texDiffuse[0]->create("Media/common/textures/lichen9.dds");
	texDiffuse[1]->create("Media/common/textures/lichen13.dds");
	texDiffuse[2]->create("Media/common/textures/lichen16.dds");
	texDiffuse[3]->create("Media/common/textures/lichen7.dds");*/
	//texDiffuse[4]->create("Media/textures/lichen10.dds");
	//texDiffuse[5]->create("Media/textures/lichen16.dds");

	/*texDiffuse[0]->create("Media/textures/lichen19.dds");
	texDiffuse[1]->create("Media/textures/lichen19.dds");
	texDiffuse[2]->create("Media/textures/lichen19.dds");
	texDiffuse[3]->create("Media/textures/lichen7.dds");
	texDiffuse[4]->create("Media/textures/lichen13.dds");
	texDiffuse[5]->create("Media/textures/lichen15.dds");*/
}

void Terrain::setTimeOfDay(float timeOfDay)
{
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
}

void Terrain::updateTerrain(float time)
{
	XMVECTOR determinant;
	XMMATRIX invTransView = XMMatrixTranspose(XMMatrixInverse(&determinant, camera->matView));

	//Set variables
	if(varView) varView->write(&invTransView);
	if(varEye) varEye->write(&camera->position);
	if(varCamView) varCamView->write(&invTransView);
	if(varCamEye) varCamEye->write(&camera->position);

	/*if(varTime)
	{
		float time = Timer::get()->getTime();
		varTime->write(&time);
	}*/

	//Fetch some data from the frame and calculate new constants
	if(varCamFrameData)
	{
		SBFrameData* fd = reinterpret_cast<SBFrameData*>(varCamFrameData->map());
		if(!fd) return;

		float minDist = fd->minDistance;
		float maxDist = fd->maxDistance;

		if(curFarDist < maxDist)
			curFarDist = maxDist;
		if(curFarDist / maxDist > 1.8f) //40% smaller
			curFarDist = maxDist;

		const float MIN_DEFAULT = 5000.0f;
		const float MAX_DEFAULT = 2.0f;
		//const float MINIMAL_DIFFERENCE = 10.0f;

		fd->minDistance = MIN_DEFAULT;
		fd->maxDistance = MAX_DEFAULT;

		varCamFrameData->unmap();

		if(varMinDistance && varMaxDistance)
		{
			varMinDistance->write(&minDist);
			varMaxDistance->write(&curFarDist);
		}
	}
}

struct CellDistanceStruct
{
	float nearz;
	float farz;
};

float Terrain::getDepth(int x, int y)
{
	if(x < 0) x = 0;
	if(x >= CAMERA_VIEW_RES) x = CAMERA_VIEW_RES - 1;
	if(y < 0) y = 0;
	if(y >= CAMERA_VIEW_RES) y = CAMERA_VIEW_RES - 1;
	return cameraView[y * CAMERA_VIEW_RES + x].depth;
}

float Terrain::getDepthInterp(int x, int y)
{
	if(x < 0)
	{
		float m = getDepth(x + 1, y);
		float d = getDepth(x + 2, y) - m;
		return m - d;
	}

	if(x >= CAMERA_VIEW_RES)
	{
		float m = getDepth(x - 1, y);
		float d = getDepth(x - 2, y) - m;
		return m - d;
	}

	if(y < 0)
	{
		float m = getDepth(x, y + 1);
		float d = getDepth(x, y + 2) - m;
		return m - d;
	}

	if(y >= CAMERA_VIEW_RES)
	{
		float m = getDepth(x, y - 1);
		float d = getDepth(x, y - 2) - m;
		return m - d;
	}

	return getDepth(x, y);
}

void Terrain::setTargetDepths()
{
	const int tileRadius = 2;

	//CellDistanceStruct depth[CAMERA_VIEW_ELEMENTS / 2];
	CellDistanceStruct depth[CAMERA_VIEW_ELEMENTS];
	for(int x = 0; x < CAMERA_VIEW_ELEMENTS; x++)
	{
		int i = x;// / 2;

		int xpos = x % CAMERA_VIEW_RES;
		int ypos = x / CAMERA_VIEW_RES;

		float dmin = getDepthInterp(xpos, ypos);
		float dmax = dmin;
		for(int xp = -tileRadius; xp <= tileRadius; xp++)
		{
			for(int yp = -tileRadius; yp <= tileRadius; yp++)
			{
				float d = getDepthInterp(xpos + xp, ypos + yp);
				dmin = std::min(d, dmin);
				dmax = std::max(d, dmax);
			}
		}

		dmin = dmin * 0.96f - 0.01f;
		dmax = dmax * 1.22f + 0.4f;
		dmin = std::max(camera->getNearZ(), dmin);
		dmax = std::min(camera->getFarZ(), dmax);

		//if(x % 2 == 0)
		//{
			depth[i].nearz = dmin;
			depth[i].farz = dmax;
		//} else {
		//	depth[i].nearz2 = dmin;
		//	depth[i].farz2 = dmax;
		//}
	}

	varCellDistance->write(depth);
}

void Terrain::getCameraResults()
{
	if(varCamResults)
	{
		float* fd = reinterpret_cast<float*>(varCamResults->map());
		if(!fd) return;
		memcpy(cameraView.data(), fd, CAMERA_VIEW_ELEMENTS * sizeof(CameraVision));
		varCamResults->unmap();

		if(varCellDistance) setTargetDepths();
	}
}