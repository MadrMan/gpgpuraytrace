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

class Flyby
{
public:
	Flyby(Camera* camera);
	virtual ~Flyby();

	void fly();

	CameraVision* getCameraView() const
	{ return cameraView; }

private:
	CameraVision* cameraView;
	Camera* camera;
};