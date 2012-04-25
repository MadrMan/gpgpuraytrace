#pragma once

class Camera;
class Terrain;

const unsigned int CAMERA_VIEW_RES = 16;
const unsigned int CAMERA_THREAD_RES = 16;

CLASSALIGN class Flyby
{
public:
	ALLOCALIGN;

	Flyby(Camera* camera);
	virtual ~Flyby();

	void fly(float time, Terrain* terrain);

	void reset();

private:
	XMVECTOR target;
	XMVECTOR orgDirToTarget;
	bool resetTarget;
	float avgHeight;
	float noTargetTime;

	Camera* camera;
};