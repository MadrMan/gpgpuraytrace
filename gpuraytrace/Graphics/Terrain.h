#pragma once

#include "../Common/Settings.h"

//! Contains camera info for each raytraced pixel
struct CameraVision
{
	float x;
	float y;
	float z;
	float depth;
};

class ITexture;
class IDevice;
class ICompute;
class IShaderVariable;
class IShaderArray;
class Flyby;
class Camera;
class Noise;

//! Terrain class which handles creation and rendering of the landscape
class Terrain
{
public:
	//! Constructor
	Terrain(IDevice* device, const std::string& theme, const Mode& mode);

	//! Destructor
	virtual ~Terrain();

	//! Create a new instance the terrain
	void create();

	//! Reload the shaders
	void reload();

	//! Render 
	void render();

	//! Update
	void updateTerrain(float time);

	//! Set the time of day
	void setTimeOfDay(float timeOfDay);

	//! Set the camera used to render the terrain with
	void setCamera(Camera* camera);
	
	//! Get the array containing the view of the prepass
	std::vector<CameraVision>& getCameraView()
	{ return cameraView; }

private:
	void getCameraResults();
	void updateShaders();
	void calculateTileSizes();
	void loadTextures();
	void setTargetDepths();
	float getDepth(int x, int y);
	float getDepthInterp(int x, int y);

	std::vector<CameraVision> cameraView;

	const std::string theme;
	Camera* camera;

	IDevice* device;
	//ITexture* texDiffuse[6];
	ICompute* compute;
	ICompute* cameraCompute;

	//Noise
	Noise* noise;
	ITexture* texNoise1D;
	ITexture* texNoise2D;

	//Screen vars
	IShaderVariable* varView;
	IShaderVariable* varEye;
	IShaderVariable* varMinDistance;
	IShaderVariable* varMaxDistance;
	IShaderVariable* varTime;
	IShaderVariable* varSunDirection;
	IShaderVariable* varThreadOffset;
	IShaderArray* varCellDistance;

	//Camera equivs
	IShaderVariable* varCamView;
	IShaderVariable* varCamEye;
	IShaderVariable* varCamMinDistance;
	IShaderVariable* varCamMaxDistance;
	IShaderArray* varCamResults;
	IShaderArray* varCamFrameData;

	//Compute variables
	int threadSizeX, threadSizeY;
	int tilesX, tilesY;
	int dispatchSizeX, dispatchSizeY;
	int tileSizeX, tileSizeY;
	float curFarDist;
	const Mode& mode;
};