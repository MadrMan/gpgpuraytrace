#include <Common.h>
#include "Flyby.h"

#include "../Graphics/Camera.h"
#include "../Common/Timer.h"
#include "../Common/Logger.h"

#include <algorithm>

Flyby::Flyby(Camera* camera) : camera(camera)
{
	cameraView.resize(CAMERA_VIEW_RES * CAMERA_VIEW_RES);
	resetTarget = true;
	avgHeight = 0.0f;
}

Flyby::~Flyby()
{

}

void Flyby::reset()
{
	resetTarget = true;
}

void Flyby::fly(float time)
{
	const float CAM_SPEED_MULT = 6.0f;
	const float camSpeed = time * CAM_SPEED_MULT;

	XMVECTOR front = camera->front;
	XMVECTOR position = camera->position;
	
	XMVECTOR best;
	XMVECTOR dirToBest;
	float depthToBest;
	XMVECTOR force = XMVectorZero();

	float score = 0.0f;
	for(int y = 0; y < CAMERA_VIEW_RES; y++)
	{
		for(int x = 0; x < CAMERA_VIEW_RES; x++)
		{
			CameraVision* cv = cameraView.data() + y * CAMERA_VIEW_RES + x;
			XMVECTOR vec = XMVectorSet(cv->x, cv->y, cv->z, 0.0f);

			if(cv->depth < 0.0001f) break; //Sky is 0

			//Push them away from 
			float strength = cv->depth * cv->depth * cv->depth;
			strength = 2.0f - strength;
			if(strength > 0.0f)
			{
				force += (position - vec) * strength * 0.0005f;
			}

			//Make depth a little less deep so as to prevent a wall collision as a target
			float movedDepth = std::max(cv->depth - 0.2f, 0.0f); 

			const float PREF_DEPTH = 100.0f;
			//const float PREF_Y = 2.0f;
			float pointScore = 0.0f;
			pointScore += PREF_DEPTH - abs(movedDepth - PREF_DEPTH);
			//pointScore += (10.0f - abs(cv->y - PREF_Y)) * 50.0f;
			float heightBonus = (cv->y - avgHeight);
			pointScore += abs(heightBonus) * 10.0f;
			//pointScore -= cv->y * 2.0f; //Don't go too high

			if(pointScore > score || isnull(score))
			{
				score = pointScore;

				dirToBest = vec - position;
				depthToBest = movedDepth;
				best = position + dirToBest * 0.15f; //Go a certain distance towards the point
			}
		}
	}

	//Reset target if flyby is restarted
	float distance = 0.0f;
	if(resetTarget)
	{
		resetTarget = false;
		target = position;
		avgHeight = XMVectorGetY(position);
		noTargetTime = 0.0f;
	} else {
		distance = XMVectorGetX(XMVector3LengthEst(position - target));
	}

	//Get median depth
	auto firstIt = cameraView.begin(); 
	auto lastIt = cameraView.end(); 
	auto middleIt = firstIt + (lastIt - firstIt) / 2; 
	std::nth_element(firstIt, middleIt, lastIt, 
		[](CameraVision& cv1, CameraVision& cv2)
		{ return cv1.depth > cv2.depth; }
	);
	float medianDepth = middleIt->depth;

	//Stop trying to go there, cant pass - but we could also be looking at the sky (0.0)
	if(medianDepth > 0.01f && medianDepth < 1.5f) 
	{
		distance = 0.0f;
		score = 0.0f;
	}

	//Logger() << "Distance to target: " << distance;
	const float POINT_REACHED = CAM_SPEED_MULT * 0.3f;
	if(distance < POINT_REACHED)
	{
		if(isnull(score)) 
		{
			Logger() << "No new target found";
			depthToBest = 0.0f;
			noTargetTime += time;

			if(noTargetTime > 5.0f)
			{
				target = position - camera->front * 0.5f;
				Logger() << "Turning around cause of no target";

				noTargetTime = 0.0f;
			}
		} else {
			noTargetTime = 0.0f;

			Logger() << "Setting new target with score " << score;
			target = best;
			orgDirToTarget = XMVector3Normalize(dirToBest);

			if(depthToBest < POINT_REACHED * 4.0f)
			{
				//Make sure we're somewhat aimed at the target, other we will KEEP turning around
				float angle = XMVectorGetX(XMVector3AngleBetweenVectors(camera->front, target - position));
				Logger() << "Angle to point when trying to turn: " << angle;

				if(angle < XM_PIDIV2) //90*
				{
					//Closest point is way too close, turn around
					target = position - dirToBest * 0.5f;

					Logger() << "Turning around";
				}
			}
		}
	}

	//Calculate next position in the curve
	const float SMOOTH_AMOUNT = 1.8f;
	float distToTarget = sqrt(XMVectorGetX(XMVector3LengthEst(target - position))); 
	distToTarget -=  2.2f; //Compensate for when the target is really close at an odd angle
	float smoothPath = std::max(distToTarget * SMOOTH_AMOUNT, 0.01f); //std::min(distToTarget * 0.2f, 6.0f);
	XMVECTOR curvePoint = XMVectorCatmullRom(position, position + camera->front * smoothPath, target - orgDirToTarget * smoothPath * 0.2f, target, 0.045f);

	//Set direction to next curve point
	XMVECTOR direction = XMVector3Normalize(curvePoint - position);

	//Always look at target
	camera->front = direction;

	//But move according to the set force
	force += direction; //Push towards target
	camera->position = XMVectorAdd(position, XMVector3Normalize(force) * camSpeed);

	//Update average height
	float avgSmoother = 1.0f - time * 0.1f;
	avgHeight = avgHeight * avgSmoother + XMVectorGetY(camera->position) * (1.0f - avgSmoother);
}