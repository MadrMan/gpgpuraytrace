#pragma once
#include <Common.h>
#include "./Factories/IWindow.h"

struct Landscape
{
	std::string name;
};


struct Mode
{
	WindowSettings ws;
	bool recordMode;
	bool fixedFrameRate;
	bool randomLandscape;
	bool enableManager;
	bool incrementDayTime;
	bool autoFlyby;
};

const Mode MODE_BENCHMARK = {
	{ 480, 270, false }, //window settings
	//{ 800, 600, false }, 
	//{ 960, 540, false },
	//{ 1920, 1080, false },
	false, //record mode
	false, //fixed framerate
	false, //random landscape
	false, //tool enabled
	true, //incrementDayTime
	false,
};

const Mode MODE_RELEASE = {
	{ 480, 270, false }, //window settings
	false, //record mode
	false, //fixed framerate
	true, //random landscape
	false, //tool disabled
	true, //incrementDayTime
	false,
};