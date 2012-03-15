#include <Common.h>
#include "Flyby.h"

#include "../Graphics/Camera.h"
#include "../Common/Timer.h"

Flyby::Flyby(Camera* camera) : camera(camera)
{
	cameraView = new CameraVision[CAMERA_VIEW_RES * CAMERA_VIEW_RES];
}

Flyby::~Flyby()
{

}

void Flyby::fly()
{
	float c = Timer::get()->getConstant();

	XMVECTOR front = camera->front;
	XMVECTOR position = camera->position;
	XMVECTOR target = position + front;

	float depth = 0.0f;
	for(int y = 0; y < CAMERA_VIEW_RES; y++)
	{
		for(int x = 0; x < CAMERA_VIEW_RES; x++)
		{
			CameraVision* cv = cameraView + y * CAMERA_VIEW_RES + x;
			if(cv->depth > depth)
			{
				depth = cv->depth;
				target = XMVectorSet(cv->x, cv->y, cv->z, 0.0f);
			}
		}
	}

	camera->front = XMVector3Normalize(target - position);
	camera->position = XMVectorAdd(position, camera->front * c);
}