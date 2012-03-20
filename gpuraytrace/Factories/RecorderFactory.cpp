#include <Common.h>
#include "RecorderFactory.h"

#include "../Adapters/RecorderWinAPI.h"

IRecorder* RecorderFactory::construct(IDevice* device, int frameRate)
{
	IRecorder* recorder = nullptr;

#ifdef _WIN32
	recorder = new RecorderWinAPI(device, frameRate);
#endif

	if(recorder)
	{
		create(&recorder);
	}

	return recorder;
}