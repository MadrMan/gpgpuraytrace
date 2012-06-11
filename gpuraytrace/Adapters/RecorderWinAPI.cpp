#include <Common.h>
#include "RecorderWinAPI.h"

#include "../Common/Logger.h"
#include "../Adapters/DeviceDirect3D.h"
#include "../Common/Timer.h"

//#include <D3D9Types.h>
#include <InitGuid.h>

#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include <Ks.h>
#include <Codecapi.h>
//#include <DShow.h>

#define STRSTR(x) #x
#define CHECKHR(x) if(FAILED(hr = (x))) \
	{ \
		DWORD last = GetLastError(); \
		if(hr == E_FAIL) \
			{ LOGERROR(last, STRSTR((x)) << " with E_FAIL"); } \
		else \
			{ LOGERROR(hr, STRSTR((x))); } \
	}

//DEFINE_MEDIATYPE_GUID(MFVideoFormat_DXGI_R8G8B8A8, DXGI_FORMAT_R8G8B8A8_UNORM);
//DEFINE_MEDIATYPE_GUID(MFVideoFormat_DXGI_R8G8B8A8, D3DFMT_X8R8G8B8);

RecorderWinAPI::RecorderWinAPI(IDevice* device, int frameRate, bool fixedSpeed) : device(device), frameRate(frameRate), fixedSpeed(fixedSpeed)
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

	IMFAttributes* sinkAttributes = nullptr;
	MFCreateAttributes(&sinkAttributes, 0);
	sinkAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);

	IMFMediaType* pMediaTypeOut = nullptr;   
	IMFMediaType* pMediaTypeIn = nullptr;  
	HRESULT hr = MFCreateSinkWriterFromURL(L"output.wmv", nullptr, sinkAttributes, &pSinkWriter);
	if(FAILED(hr)) 
	{
		switch(hr){
			case MF_E_NOT_FOUND:
			LOGFUNCERROR("SetInputMediaType returned MF_E_INVALIDMEDIATYPE");
			break;
		default:
			LOGERROR(hr, "MFCreateSinkWriterFromURL");
		}
			return false;
	}

	const GUID exportFormat = MFVideoFormat_WMV3;	//MFVideoFormat_MPEG2; //MFVideoFormat_WMV3; //MFVideoFormat_H264
	const GUID importFormat = MFVideoFormat_RGB32; //MFVideoFormat_DXGI_R8G8B8A8; //MFVideoFormat_RGB32

	//Enumerate available encoders
	/*IMFActivate** activators;
	UINT32 activatorCount;
	MFT_REGISTER_TYPE_INFO info = { 0 };
	info.guidMajorType = MFMediaType_Video;
	info.guidSubtype = exportFormat;

	IMFTransform* outputTransform = nullptr;
	if(FAILED(hr = MFTEnumEx(MFT_CATEGORY_VIDEO_ENCODER, 
		MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_ASYNCMFT | MFT_ENUM_FLAG_HARDWARE | MFT_ENUM_FLAG_TRANSCODE_ONLY,
		nullptr,
		&info,
		&activators,
		&activatorCount)))
	{
		LOGERROR(hr, "MFTEnumEx");
	} else {
		for(UINT32 x = 0; x < activatorCount; x++)
		{
			IMFActivate* act = activators[x];
			if(!outputTransform)
			{
				WCHAR* szFriendlyName = nullptr;
				UINT cchName = 0;

				hr = act->GetAllocatedString(
					MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
					&szFriendlyName, &cchName);

				if (SUCCEEDED(hr))
				{
					Logger() << szFriendlyName;
				}
				CoTaskMemFree(szFriendlyName);

				IMFTransform** pOutputTransform = &outputTransform;
				hr = act->ActivateObject(IID_PPV_ARGS(pOutputTransform));
				if(FAILED(hr))
				{
					LOGERROR(hr, "ActivateObject");
				}
			}
			act->Release();
		}
		CoTaskMemFree(activators);
	}*/

	//Set the output media type.
	CHECKHR(MFCreateMediaType(&pMediaTypeOut));   
	CHECKHR(pMediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));     
    CHECKHR(pMediaTypeOut->SetGUID(MF_MT_SUBTYPE, exportFormat));

    CHECKHR(pMediaTypeOut->SetUINT32(MF_MT_AVG_BITRATE, 12 * 1024 * 1024)); //16mb
	//CHECKHR(pMediaTypeOut->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_Main));

	//CHECKHR(pMediaTypeOut->SetUINT32(CODECAPI_AVEncCommonRateControlMode, eAVEncCommonRateControlMode_UnconstrainedVBR));
	//CHECKHR(pMediaTypeOut->SetUINT32(CODECAPI_AVEncCommonRateControlMode, eAVEncCommonRateControlMode_CBR));
	//CHECKHR(pMediaTypeOut->SetUINT32(CODECAPI_AVEncCommonQuality, 95));
	//ICodecAPI* h264Codec = nullptr;
	//pMediaTypeOut->QueryInterface(IID_ICodecAPI, (void**)&h264Codec);

    CHECKHR(pMediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
    CHECKHR(MFSetAttributeSize(pMediaTypeOut, MF_MT_FRAME_SIZE, width, height));
    CHECKHR(MFSetAttributeRatio(pMediaTypeOut, MF_MT_FRAME_RATE, frameRate, 1));
    CHECKHR(MFSetAttributeRatio(pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, 1, 1));
    CHECKHR(pSinkWriter->AddStream(pMediaTypeOut, &streamIndex));

	//Set the input media type.
    CHECKHR(MFCreateMediaType(&pMediaTypeIn));
    CHECKHR(pMediaTypeIn->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
    CHECKHR(pMediaTypeIn->SetGUID(MF_MT_SUBTYPE, importFormat)); 
    CHECKHR(pMediaTypeIn->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
    CHECKHR(MFSetAttributeSize(pMediaTypeIn, MF_MT_FRAME_SIZE, width, height));
    CHECKHR(MFSetAttributeRatio(pMediaTypeIn, MF_MT_FRAME_RATE, frameRate, 1)); 
    CHECKHR(MFSetAttributeRatio(pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1));
    hr = pSinkWriter->SetInputMediaType(streamIndex, pMediaTypeIn, NULL);
	if(FAILED(hr))
	{
		switch(hr)
		{
		case MF_E_INVALIDMEDIATYPE:
			LOGFUNCERROR("SetInputMediaType returned MF_E_INVALIDMEDIATYPE");
			break;
		case MF_E_INVALIDSTREAMNUMBER:
			LOGFUNCERROR("SetInputMediaType returned MF_E_INVALIDSTREAMNUMBER");
			break;
		case MF_E_TOPO_CODEC_NOT_FOUND:
			LOGFUNCERROR("SetInputMediaType returned MF_E_TOPO_CODEC_NOT_FOUND");	
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
	CHECKHR(MFFrameRateToAverageTimePerFrame(frameRate, 1, &rtDuration));

	static_cast<DeviceDirect3D*>(device)->setRecorder(this);

	return true;
}

void RecorderWinAPI::start()
{
	IRecorder::start();

	HRESULT hr;
	CHECKHR(pSinkWriter->BeginWriting());
	if(FAILED(hr))
	{
		DWORD err = GetLastError();
	}
}

void RecorderWinAPI::stop()
{
	IRecorder::stop();

	HRESULT hr;
	CHECKHR(pSinkWriter->Finalize());
}

void RecorderWinAPI::write(void* frame, int stride)
{
    IMFSample *pSample = NULL;
	HRESULT hr;

    const DWORD cbWidth = sizeof(DWORD) * width;
    const DWORD cbTotal = cbWidth * height;

    // Create a new memory buffer.
    if(!pBuffer) CHECKHR(MFCreateMemoryBuffer(cbTotal, &pBuffer));

    // Lock the buffer and copy the video frame to the buffer.
	BYTE *pData = NULL;
    CHECKHR(pBuffer->Lock(&pData, NULL, NULL));

    //CHECKHR(MFCopyImage(pData, cbWidth, (BYTE*)frame, stride, cbWidth, height));

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			DWORD dwc = *((DWORD*)((char*)frame + y * stride) + x);
			//DWORD dwc = *((DWORD*)((char*)frame + (height - y - 1) * stride) + x);
			DWORD* dw = ((DWORD*)pData) + y * width + x;

			*dw = dwc & 0x0000FF00 | (dwc & 0x000000FF) << 16 | (dwc & 0x00FF0000) >> 16;
		}
	}

	CHECKHR(pBuffer->Unlock());

	//Set length of valid data
	CHECKHR(pBuffer->SetCurrentLength(cbTotal));

    // Create a media sample and add the buffer to the sample.
    CHECKHR(MFCreateSample(&pSample));
	CHECKHR(pSample->AddBuffer(pBuffer));

	UINT64 thisFrameDuration = rtDuration;
	if(!fixedSpeed)
	{
		float modifier = Timer::get()->getConstant();
		thisFrameDuration = (UINT64)(10000000.0f * modifier);
	}

    // Set the time stamp and the duration.
    CHECKHR(pSample->SetSampleTime(rtStart));
    CHECKHR(pSample->SetSampleDuration(thisFrameDuration));
	rtStart += thisFrameDuration;

    // Send the sample to the Sink Writer.
    CHECKHR(pSinkWriter->WriteSample(streamIndex, pSample));

    pSample->Release();
}