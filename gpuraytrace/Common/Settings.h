#pragma once
#include "./Factories/IWindow.h"

struct Mode
{
	WindowSettings ws;
	bool recordMode;
	bool fixedFrameRate;
	bool randomLandscape;
	bool enableManager;
	bool incrementDayTime;
};

const Mode MODE_BENCHMARK = {
	{ 480, 270, false }, //window settings
	//{ 800, 600, false }, 
	//{ 960, 540, false },
	//{ 1920, 1080, false },
	false, //record mode
	false, //fixed framerate
	false, //random landscape
	true, //tool enabled
	false //incrementDayTime
};

const Mode MODE_RECORD = {
	{ 1920, 1080, true },
	true, //record mode
	true, //fixed framerate
	true, //random landscape
	false, //tool enabled
	true //incrementDayTime
};

const Mode MODE_RELEASE = {
	{ 480, 270, false }, //window settings
	false, //record mode
	false, //fixed framerate
	true, //random landscape
	false, //tool enabled
	true //incrementDayTime
};