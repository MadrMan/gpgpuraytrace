#include <Common.h>
#include "DeviceDirect3D.h"

#include "WindowWinAPI.h"
#include "ComputeDirect3D.h"
#include "TextureDirect3D.h"

#include "../Common/Logger.h"
#include "../Adapters/RecorderWinAPI.h"

#include <D3Dcompiler.h>
#include <fstream>

DeviceDirect3D::DeviceDirect3D(IWindow* window) : IDevice(DeviceAPI::Direct3D, window)
{
	device = nullptr;
	context = nullptr;
	swapChain = nullptr;
	swapBackBuffer = nullptr;
	swapBackBufferSRV = nullptr;
	uavSwapBuffer = nullptr;

	recorder = nullptr;
	swapStaging = nullptr;
}

DeviceDirect3D::~DeviceDirect3D()
{
	if(swapStaging) delete swapStaging;

	if(uavSwapBuffer) uavSwapBuffer->Release();
	if(device) device->Release();
	if(context) context->Release();
	if(swapChain) swapChain->Release();
	if(swapBackBuffer) swapBackBuffer->Release();
}

bool DeviceDirect3D::getAdapterHandle(std::vector<IDXGIAdapter1*>* adapters)
{
	HRESULT hr;

	//Create DXGI factory
	IDXGIFactory1* dxgiFactory;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&dxgiFactory));
	if(FAILED(hr))
	{
		LOGERROR(hr, "CreateDXGIFactory1");
		return false;
	}

	//Get all the adapters
	UINT i = 0;
	IDXGIAdapter1* pAdapter = nullptr;
	while(dxgiFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{ 
		adapters->push_back(pAdapter);

		DXGI_ADAPTER_DESC1 desc;
		pAdapter->GetDesc1(&desc);
		std::wstring descriptionw(desc.Description);
		std::string description(descriptionw.begin(), descriptionw.end());
		Logger() << "Adapter found: (" << i << ") " << description;

		++i;
	}
	dxgiFactory->Release();

	if(adapters->empty())
	{
		LOGFUNCERROR("Your graphics card does not appear to support DirectX 10 or later");
		return false;
	}

	return true;
}

bool DeviceDirect3D::create()
{
	HMODULE libD3D11 = LoadLibrary("d3d11.dll");
	if(!libD3D11)
	{
		Logger() << "Could not load d3d11.dll, you probably do not have DirectX 11 installed.";
		return false;
	}

	HMODULE libCompiler43 = LoadLibrary("d3dcompiler_43.dll");
	if(!libCompiler43)
	{
		Logger() << "Could not load d3dcompiler_43.dll, try updating your DirectX";
		return false;
	}

	//Release handles
	FreeLibrary(libD3D11);
	FreeLibrary(libCompiler43);

	std::vector<IDXGIAdapter1*> adapters;
	if(!getAdapterHandle(&adapters))
	{
		return false;
	}

	UINT createDeviceFlags = 0;
#if defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 /*, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0*/ };
	DXGI_SWAP_CHAIN_DESC sd;
	
	ZeroMemory(&sd,sizeof(sd));
	
	const WindowSettings& ws = getWindow()->getWindowSettings();

	sd.BufferCount = 1;	
	sd.BufferDesc.Width = (UINT)ws.width;
	sd.BufferDesc.Height = (UINT)ws.height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //_SRGB;
	sd.BufferDesc.RefreshRate.Numerator = 60;	
	sd.BufferDesc.RefreshRate.Denominator = 1;	
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.OutputWindow = static_cast<WindowWinAPI*>(getWindow())->getHandle();
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = ws.fullscreen ? FALSE : TRUE;
	
	int selectedAdapterId = ws.gpu;
	IDXGIAdapter* selectedAdapter = nullptr;
	if(selectedAdapterId >= 0) 
	{
		if(selectedAdapterId < (int)adapters.size())
		{
			selectedAdapter = adapters[selectedAdapterId];
		} else {
			LOGFUNCERROR("Selected graphics card " << selectedAdapterId << " does not exist");
		}
	}

	HRESULT result = D3D11CreateDeviceAndSwapChain(selectedAdapter, selectedAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, featureLevels,
                        _countof(featureLevels), D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &context);

	if(result != S_OK)
	{
		if(result == DXGI_ERROR_UNSUPPORTED)
		{
			LOGFUNCERROR("Your videocard does not appear to support DirectX 11");
		} else {
			LOGERROR(result, "D3D11CreateDeviceAndSwapChain");
		}
		return false;
	}

	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;	
	/*result = device->CreateShaderResourceView(swapBackBuffer, 0, &swapBackBufferSRV);

	if(result != S_OK){
		LOGERROR(result, "ID3D11Device::CreateShaderResourceView");
		return false;
	}*/

	D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS dxHwOpt;
	result = device->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &dxHwOpt, sizeof(dxHwOpt));
	if(FAILED(result))
	{
		LOGERROR(result, "CheckFeatureSupport");
		return false;
	}
	if(!dxHwOpt.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
	{
		Logger() << "ComputeShaders are not supported on this device";
		return false;
	}
	
	//Get the buffer from the swapchain
	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&swapBackBuffer);
	if(result != S_OK)
    {
		LOGERROR(result, "IDXGISwapChain::GetBuffer");
        return false;
    }

	//Create trace result texture/RT
	swapStaging = static_cast<TextureDirect3D*>(createTexture());
	swapStaging->create(TextureDimensions::Texture2D, TextureFormat::R8G8B8A8_UNORM, sd.BufferDesc.Width, sd.BufferDesc.Height, nullptr, TextureBinding::Staging, CPUAccess::Read);
	/*result = device->CreateRenderTargetView(traceResultTexture->getResource(), nullptr, &traceResultRT);
	if(FAILED(result))
	{
		LOGERROR(result, "CreateRenderTargetView");
		return false;
	}*/

	//Create the UAV for the trace result
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = sd.BufferDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Buffer.FirstElement = 0; 
	uavDesc.Buffer.NumElements = sd.BufferDesc.Width * sd.BufferDesc.Height;

	result = device->CreateUnorderedAccessView(swapBackBuffer, &uavDesc, &uavSwapBuffer);
	if(FAILED(result))
	{
		LOGERROR(result, "CreateUnorderedAccessView");
		return false;
	}

	//Setup sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; //D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	ID3D11SamplerState* sampler;
	device->CreateSamplerState(&samplerDesc, &sampler);
	context->CSSetSamplers(0, 1, &sampler);
	sampler->Release();

	return true;
}

void DeviceDirect3D::setRecorder(RecorderWinAPI* recorder)
{
	this->recorder = recorder;
}

void DeviceDirect3D::present()
{
	HRESULT result = swapChain->Present(0, 0);
	if(FAILED(result))
	{
		LOGERROR(result, "Present");
	}

	if(recorder && recorder->isRecording())
	{
		ID3D11Resource* resource = swapStaging->getResource();
		context->CopyResource(resource, swapBackBuffer);

		D3D11_MAPPED_SUBRESOURCE mapped;
		result = context->Map(resource, 0, D3D11_MAP_READ, 0, &mapped);
		if(SUCCEEDED(result))
		{
			recorder->write(mapped.pData, mapped.RowPitch);
			context->Unmap(resource, 0);
		} else {
			LOGERROR(result, "Map");
		}
	}
}

void DeviceDirect3D::flush()
{
	context->Flush();
}

ICompute* DeviceDirect3D::createCompute()
{
	return new ComputeDirect3D(this);
}

ITexture* DeviceDirect3D::createTexture()
{
	return new TextureDirect3D(this);
}