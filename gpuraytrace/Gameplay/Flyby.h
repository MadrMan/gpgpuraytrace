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

	CameraVision* getCameraView() const
	{ return cameraView; }

	void reset();

private:
	XMVECTOR target;
	XMVECTOR lastTarget;
	bool resetTarget;

	CameraVision* cameraView;
	Camera* camera;
};