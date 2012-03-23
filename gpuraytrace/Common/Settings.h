#pragma
#include "./Factories/IWindow.h"

struct Mode
{
	WindowSettings ws;
	bool recordMode;
	bool fixedFrameRate;
	bool randomLandscape;
};

const Mode MODE_TEST = {
	{ 480, 270, false }, //window settings
	//{ 800, 600, false }, 
	//{ 960, 540, false },
	//{ 1920, 1080, false },
	false, //record mode
	false, //fixed framerate
	false //random landscape
};

const Mode MODE_RECORD = {
	{ 1920, 1080, true },
	true, //record mode
	true, //fixed framerate
	true //random landscape
};
