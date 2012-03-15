#include <Common.h>
#include "Flyby.h"

#include "../Graphics/Camera.h"
#include "../Common/Timer.h"

Flyby::Flyby(Camera* camera) : camera(camera)
{
	cameraView = new CameraVision[CAMERA_VIEW_RES * CAMERA_VIEW_RES];
	resetTarget = true;
}

Flyby::~Flyby()
{

}

void Flyby::reset()
{
	resetTarget = true;
	lastTarget = camera->position;
}

void Flyby::fly()
{
	float c = Timer::get()->getConstant();

	XMVECTOR front = camera->front;
	XMVECTOR position = camera->position;
	
	XMVECTOR best;
	XMVECTOR force = XMVectorZero();

	float depth = 0.0f;
	for(int y = 0; y < CAMERA_VIEW_RES; y++)
	{
		for(int x = 0; x < CAMERA_VIEW_RES; x++)
		{
			CameraVision* cv = cameraView + y * CAMERA_VIEW_RES + x;
			float cvdepth = cv->depth;
			if(cvdepth < 0.1f) //Too close
			{
				force += (position - XMVectorSet(cv->x, cv->y, cv->z, 0.0f)) * 0.01f;
			}
			if(cvdepth > depth && cvdepth < 200.0f)
			{
				depth = cv->depth;
				XMVECTOR dirToPoint = XMVectorSet(cv->x, cv->y, cv->z, 0.0f) - position;
				best = position + dirToPoint * 0.2f; //Go a certain distance towards the point
			}
		}
	}

	float distance = XMVectorGetX(XMVector3LengthEst(position - target));
	if(resetTarget)
	{
		distance = 0.0f;
		resetTarget = false;
	}

	if(distance < 5.0f)
	{
		target = best;
	}

	//XMVectorHermite / XMVectorCatmullRom 

	//Push force towards target
	XMVECTOR direction = XMVector3Normalize(target - position); 

	//Always look at target
	camera->front = direction;

	//But move according to the set force
	force += direction; //Push towards target
	camera->position = XMVectorAdd(position, XMVector3Normalize(force) * c);
}