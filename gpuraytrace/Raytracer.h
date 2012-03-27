#pragma once
#include "./Common/Settings.h"

class IShaderVariable;
class IShaderArray;
class IDevice;
class IWindow;
class ICompute;
class ITexture;
class Camera;
class Noise;
class Flyby;

struct Mode;

//! Main class for the raytrace project
//! Calling run on this class will block and run the raytracing program
class Raytracer
{
public:
	//! Constructor
	Raytracer();

	//! Destructor
	virtual ~Raytracer();

	//! Main function for this class, does all the work needed to show the raytracer and update it
	//! Blocks until the raytracer exits
	void run(const Mode& mode);

	//! Reload the compute shader
	void loadComputeShader();

private:
	void updateTerrain(float times, const Mode& mode);
	void updateCompute(float time, const Mode& mode);
	void updateComputeVars();
	void runCompute();

	void calculateTileSizes();

	IDevice* device;
	IWindow* window;
	ICompute* compute;
	ICompute* cameraCompute;
	ITexture* texNoise1D;
	ITexture* texNoise2D;
	Camera* camera;
	Noise* noise;
	Flyby* flyby;

	//Texturing of the landscape
	ITexture* texDiffuse[6];

	//Screen vars
	IShaderVariable* varView;
	IShaderVariable* varEye;
	IShaderVariable* varMinDistance;
	IShaderVariable* varMaxDistance;
	IShaderVariable* varTime;
	IShaderVariable* varSunDirection;
	IShaderVariable* varThreadOffset;

	//Camera equivs
	IShaderVariable* varCamView;
	IShaderVariable* varCamEye;
	IShaderVariable* varCamMinDistance;
	IShaderVariable* varCamMaxDistance;
	IShaderArray* varCamResults;
	IShaderArray* varCamFrameData;

	float timeOfDay;
	float timeOfYear;

	int threadSizeX, threadSizeY;
	int tilesX, tilesY;
	int dispatchSizeX, dispatchSizeY;
	int tileSizeX, tileSizeY;
};