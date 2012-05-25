#pragma once
#include <Common.h>
#include "./Factories/IWindow.h"

struct Landscape
{
	std::string name;
};

const std::array<Landscape, 3> landscapes = { "nomadplains","greenrocks", "testing" };	//TODO create from directory
const float STEP_MOD_NORMAL = 1.009f;	
const float STEP_MOD_RECORD = 1.001f;	

struct Mode
{
	WindowSettings ws;
	bool recordMode;
	bool fixedFrameRate;
	bool randomLandscape;
	bool enableManager;
	bool incrementDayTime;
	float step_mod;
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
	false, //incrementDayTime
	STEP_MOD_NORMAL
};

const Mode MODE_RELEASE = {
	{ 480, 270, false }, //window settings
	false, //record mode
	false, //fixed framerate
	true, //random landscape
	false, //tool disabled
	true, //incrementDayTime
	STEP_MOD_NORMAL
};