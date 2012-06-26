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
class Terrain;

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
	void run(const Mode& mode, const std::string landscape);

	//! Reload the terrain
	void reloadTerrain();

private:
	void updateCompute(float time, const Mode& mode);

	IDevice* device;
	IWindow* window;
	Camera* camera;
	Flyby* flyby;
	Terrain* terrain;

	float timeOfDay;
	float timeOfYear;
};