#include <Common.h>
#include "Flyby.h"

#include "../Graphics/Camera.h"
#include "../Common/Timer.h"
#include "../Common/Logger.h"

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
}

void Flyby::fly()
{
	const float c = Timer::get()->getConstant();
	const float camSpeed = c * 5.0f;

	XMVECTOR front = camera->front;
	XMVECTOR position = camera->position;
	
	XMVECTOR best;
	XMVECTOR dirToBest;
	XMVECTOR force = XMVectorZero();

	float score = -10000.0f;
	for(int y = 0; y < CAMERA_VIEW_RES; y++)
	{
		for(int x = 0; x < CAMERA_VIEW_RES; x++)
		{
			CameraVision* cv = cameraView + y * CAMERA_VIEW_RES + x;
			float cvDepth = cv->depth;
			XMVECTOR vec = XMVectorSet(cv->x, cv->y, cv->z, 0.0f);

			//Push them away from 
			float strength = cvDepth * cvDepth * cvDepth;
			strength = 2.0f - strength;
			if(strength > 0.0f)
			{
				force += (position - vec) * strength * 0.001f;
			}

			const float PREF_DEPTH = 200.0f;
			const float PREF_Y = 2.0f;
			float pointScore = 0.0f;
			pointScore += PREF_DEPTH - abs(cvDepth - PREF_DEPTH);
			pointScore += (10.0f - abs(cv->y - PREF_Y)) * 50.0f;

			if(pointScore > score)
			{
				score = pointScore;

				dirToBest = vec - position;
				best = position + dirToBest * 0.2f; //Go a certain distance towards the point
			}
		}
	}

	float distance = 0.0f;
	if(resetTarget)
	{
		resetTarget = false;
		target = position;
	} else {
		distance = XMVectorGetX(XMVector3LengthEst(position - target));
	}
	
	Logger() << "Distance to target: " << distance;
	if(distance < camSpeed * 5.0f)
	{
		Logger() << "Setting new target with score " << score;
		target = best;
		orgDirToTarget = XMVector3Normalize(dirToBest);
	}

	//XMVectorHermite / XMVectorCatmullRom 

	//Calculate next position in the curve
	

	//Push force towards target
	float distToTarget = XMVectorGetX(XMVector3LengthEst(target - position)); 
	float smoothPath = std::min(distToTarget, 5.0f);
	XMVECTOR curvePoint = XMVectorCatmullRom(position, position + camera->front * smoothPath, target - orgDirToTarget, target, 0.1f);

	//Set direction to next curve point
	XMVECTOR direction = XMVector3Normalize(curvePoint - position);

	//Always look at target
	camera->front = direction;

	//But move according to the set force
	force += direction; //Push towards target
	camera->position = XMVectorAdd(position, XMVector3Normalize(force) * camSpeed);
}