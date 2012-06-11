#include <Common.h>
#include "RecorderFactory.h"

#include "../Adapters/RecorderWinAPI.h"

IRecorder* RecorderFactory::construct(IDevice* device, int frameRate, bool fixedSpeed)
{
	IRecorder* recorder = nullptr;

#ifdef _WIN32
	recorder = new RecorderWinAPI(device, frameRate, fixedSpeed);
#endif

	if(recorder)
	{
		create(&recorder);
	}

	return recorder;
}