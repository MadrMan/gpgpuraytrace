#pragma once

class IShaderVariable;
class IShaderArray;
class IDevice;
class IWindow;
class ICompute;
class ITexture;
class Camera;
class Noise;
class Flyby;

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
	void run();

	//! Reload the compute shader
	void loadComputeShader();

	void updateCompute();
	void updateComputeVars();

private:
	void updateTerrain();

	IDevice* device;
	IWindow* window;
	ICompute* compute;
	ICompute* cameraCompute;
	ITexture* texNoise1D;
	ITexture* texNoise2D;
	Camera* camera;
	Noise* noise;
	Flyby* flyby;

	//Screen vars
	IShaderVariable* varView;
	IShaderVariable* varEye;
	IShaderArray* varFrameData;
	IShaderVariable* varMinDistance;
	IShaderVariable* varMaxDistance;
	IShaderVariable* varTime;

	//Camera equivs
	IShaderVariable* varCamView;
	IShaderVariable* varCamEye;
	IShaderVariable* varCamMinDistance;
	IShaderVariable* varCamMaxDistance;
	IShaderArray* varCamResults;

	float curMinDistance;
	float curMaxDistance;
};