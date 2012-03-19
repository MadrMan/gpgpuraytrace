#include <Common.h>
#include "RecorderFactory.h"

#include "../Adapters/RecorderWinAPI.h"

IRecorder* RecorderFactory::construct(IDevice* device)
{
	IRecorder* recorder = nullptr;

#ifdef _WIN32
	recorder = new RecorderWinAPI(device);
#endif

	if(recorder)
	{
		create(&recorder);
	}

	return recorder;
}