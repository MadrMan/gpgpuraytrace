#pragma once

class Camera;
class Terrain;

const unsigned int CAMERA_VIEW_RES = 32;
const unsigned int CAMERA_THREAD_RES = 16;

//! Class which handles autonomous flying trough the terrain
CLASSALIGN class Flyby
{
public:
	ALLOCALIGN;

	//! Constructor
	Flyby(Camera* camera);

	//! Destructor
	virtual ~Flyby();

	//! Update
	void fly(float time, Terrain* terrain);

	//! Acquire a new target
	//! \note Call this when the camera has been moved from the outside
	void reset();

private:
	XMVECTOR target;
	XMVECTOR orgDirToTarget;
	bool resetTarget;
	float avgHeight;
	float noTargetTime;

	Camera* camera;
};