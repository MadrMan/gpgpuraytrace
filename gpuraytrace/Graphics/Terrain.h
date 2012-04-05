#pragma once

#include "../Common/Settings.h"

class ITexture;
class IDevice;
class ICompute;
class IShaderVariable;
class IShaderArray;
class Flyby;
class Camera;
class Noise;
class Terrain
{
public:
	Terrain(IDevice* device, const std::string& theme);
	virtual ~Terrain();

	void create(const Mode& mode);
	void reload();
	void render();
	void updateTerrain(float time, const Mode& mode);
	void getFlybyData(Flyby* flyby);
	void setTimeOfDay(float timeOfDay);
	void setCamera(Camera* camera);

private:
	void updateShaders();
	void calculateTileSizes();
	void loadTextures();

	const std::string theme;
	Camera* camera;

	IDevice* device;
	ITexture* texDiffuse[6];
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
};