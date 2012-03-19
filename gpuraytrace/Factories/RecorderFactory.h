#pragma once

#include "IRecorder.h"
#include "Factory.h"

//! Factory used to create new devices
class RecorderFactory : private Factory
{
public:
	//! Construct a new recorder
	//! \return nullptr if there was an error, otherwise a valid recorder
	static IRecorder* construct(IDevice* device);
};