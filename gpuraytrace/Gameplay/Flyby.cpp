#include <Common.h>
#include "Flyby.h"

#include "../Graphics/Camera.h"
#include "../Common/Timer.h"

Flyby::Flyby(Camera* camera) : camera(camera)
{

}

Flyby::~Flyby()
{

}

void Flyby::fly()
{
	float c = Timer::get()->getConstant();

	XMVECTOR front = camera->front;
	XMVECTOR position = camera->position;

	camera->position = XMVectorAdd(position, front * c);
}