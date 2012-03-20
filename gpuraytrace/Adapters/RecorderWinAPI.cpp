#include <Common.h>
#include "RecorderWinAPI.h"

#include "../Common/Logger.h"
#include "../Adapters/DeviceDirect3D.h"

#include <D3D9Types.h>
#include <InitGuid.h>

//DEFINE_MEDIATYPE_GUID(MFVideoFormat_DXGI_R8G8B8A8, DXGI_FORMAT_R8G8B8A8_UNORM);
DEFINE_MEDIATYPE_GUID(MFVideoFormat_DXGI_R8G8B8A8, D3DFMT_X8R8G8B8);

RecorderWinAPI::RecorderWinAPI(IDevice* device) : device(device)
{
	width = device->getWindow()->getWindowSettings().width;
	height = device->getWindow()->getWindowSettings().height;

	pSinkWriter = nullptr;
	pBuffer = nullptr;
	streamIndex = 0;

	rtStart = 0;
	rtDuration = 0;
}

RecorderWinAPI::~RecorderWinAPI()
{
	if(pBuffer) pBuffer->Release();
    if(pSinkWriter) pSinkWriter->Release();

	static_cast<DeviceDirect3D*>(device)->setRecorder(nullptr);
}

bool RecorderWinAPI::initialize()
{
	static bool isInitialized = false;
	if(!isInitialized)
	{
		isInitialized = true;

		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		MFStartup(MF_VERSION);
	}

	//MFShutdown();
    //CoUninitialize();

	return true;
}

bool RecorderWinAPI::create()
{
	initialize();

	IMFMediaType* pMediaTypeOut = nullptr;   
	IMFMediaType* pMediaTypeIn = nullptr;  
	HRESULT hr = MFCreateSinkWriterFromURL(L"output.wmv", NULL, NULL, &pSinkWriter);
	if(FAILED(hr)) 
	{
		LOGERROR(hr, "MFCreateSinkWriterFromURL");
		return false;
	}

	const UINT32 VIDEO_FPS = 25;
	

	//Set the output media type.
	hr = MFCreateMediaType(&pMediaTypeOut);   
	hr = pMediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);     
    hr = pMediaTypeOut->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_WMV3); //MFVideoFormat_H264 
    hr = pMediaTypeOut->SetUINT32(MF_MT_AVG_BITRATE, 800000);
    hr = pMediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    hr = MFSetAttributeSize(pMediaTypeOut, MF_MT_FRAME_SIZE, width, height);
    hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_FRAME_RATE, VIDEO_FPS, 1);
    hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    hr = pSinkWriter->AddStream(pMediaTypeOut, &streamIndex);
	if(FAILED(hr))
	{
		LOGERROR(hr, "AddStream");
		return false;
	}

	//Set the input media type.
    hr = MFCreateMediaType(&pMediaTypeIn);
    hr = pMediaTypeIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    hr = pMediaTypeIn->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_DXGI_R8G8B8A8); //MFVideoFormat_RGB32
    hr = pMediaTypeIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    hr = MFSetAttributeSize(pMediaTypeIn, MF_MT_FRAME_SIZE, width, height);
    hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_FRAME_RATE, VIDEO_FPS, 1); 
    hr = MFSetAttributeRatio(pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    hr = pSinkWriter->SetInputMediaType(streamIndex, pMediaTypeIn, NULL);
	if(FAILED(hr))
	{
		switch(hr)
		{
		case MF_E_INVALIDMEDIATYPE:
			LOGFUNCERROR("SetInputMediaType returned MF_E_INVALIDMEDIATYPE");
			break;
		default:
			LOGERROR(hr, "SetInputMediaType");
			break;
		}
		
		return false;
	}

    pMediaTypeOut->Release();
    pMediaTypeIn->Release();

	rtStart = 0;
	MFFrameRateToAverageTimePerFrame(VIDEO_FPS, 1, &rtDuration);

	static_cast<DeviceDirect3D*>(device)->setRecorder(this);

	return true;
}

void RecorderWinAPI::start()
{
	IRecorder::start();

	HRESULT hr = pSinkWriter->BeginWriting();
}

void RecorderWinAPI::stop()
{
	IRecorder::stop();

	HRESULT hr = pSinkWriter->Finalize();
}

void RecorderWinAPI::write(void* frame, int stride)
{
    IMFSample *pSample = NULL;
    IMFMediaBuffer *pBuffer = NULL;
	HRESULT hr;

    const DWORD cbWidth = sizeof(DWORD) * width;
    const DWORD cbTotal = cbWidth * height;

    // Create a new memory buffer.
    if(!pBuffer) MFCreateMemoryBuffer(cbTotal, &pBuffer);

    // Lock the buffer and copy the video frame to the buffer.
	BYTE *pData = NULL;
    hr = pBuffer->Lock(&pData, NULL, NULL);

    hr = MFCopyImage(pData, cbWidth, (BYTE*)frame, stride, cbWidth, height);
	/*for(DWORD y = 0; y < height; y++)
	{
		for(DWORD x = 0; x < cbWidth; x++)
		{
			DWORD* dw = ((DWORD*)pData) + y * cbWidth + x;
			DWORD dwo = *dw;
			*dw = dwo & 0xFF000000;
		}
	}*/

	hr = pBuffer->Unlock();

	//Set length of valid data
	hr = pBuffer->SetCurrentLength(cbTotal);

    // Create a media sample and add the buffer to the sample.
    hr = MFCreateSample(&pSample);
	hr = pSample->AddBuffer(pBuffer);

    // Set the time stamp and the duration.
    hr = pSample->SetSampleTime(rtStart);
    hr = pSample->SetSampleDuration(rtDuration);
	rtStart += rtDuration;

    // Send the sample to the Sink Writer.
    if (SUCCEEDED(hr))
    {
        hr = pSinkWriter->WriteSample(streamIndex, pSample);
    }

    pSample->Release();
}