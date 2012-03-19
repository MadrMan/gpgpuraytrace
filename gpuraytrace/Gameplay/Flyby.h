#pragma once

class Camera;

const unsigned int CAMERA_VIEW_RES = 32;

struct CameraVision
{
	float x;
	float y;
	float z;
	float depth;
};

CLASSALIGN class Flyby
{
public:
	ALLOCALIGN;

	Flyby(Camera* camera);
	virtual ~Flyby();

	void fly();

	std::vector<CameraVision>& getCameraView()
	{ return cameraView; }

	void reset();

private:
	XMVECTOR target;
	XMVECTOR orgDirToTarget;
	bool resetTarget;
	float avgHeight;

	std::vector<CameraVision> cameraView;
	Camera* camera;
};