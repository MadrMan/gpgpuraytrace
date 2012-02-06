#pragma once

#include "IDevice.h"

class Raytracer
{
public:
	Raytracer();
	virtual ~Raytracer();

	void run();

private:
	IDevice* device;
};