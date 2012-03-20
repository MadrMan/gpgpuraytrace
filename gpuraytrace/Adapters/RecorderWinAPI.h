#pragma once

#include "../Factories/IRecorder.h"

#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>

class RecorderWinAPI : public IRecorder
{
public:
	RecorderWinAPI(IDevice* device, int frameRate);
	virtual ~RecorderWinAPI();
	
	virtual bool create() override;

	virtual void start() override;
	virtual void stop() override;
	virtual void write(void* frame, int stride) override;

private:
	static bool initialize();

	IDevice* device;
	int width, height, frameRate;
	IMFSinkWriter* pSinkWriter;
	IMFMediaBuffer *pBuffer;
	DWORD streamIndex;
	UINT64 rtDuration;
	UINT64 rtStart;
};