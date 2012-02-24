#pragma once

class IShaderVariable;
class IDevice;
class IWindow;
class ICompute;
class ITexture;
class Camera;

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
	IDevice* device;
	IWindow* window;
	ICompute* compute;
	ITexture* texNoise;
	Camera* camera;

	IShaderVariable* varView;
	IShaderVariable* varProjection;
	IShaderVariable* varEye;
};