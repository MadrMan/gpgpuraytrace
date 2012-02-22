#include <Common.h>
#include "DeviceDirect3D.h"

#include "WindowWinAPI.h"
#include "ComputeDirect3D.h"

#include "../Common/Logger.h"

#include <D3Dcompiler.h>
#include <fstream>

DeviceDirect3D::DeviceDirect3D(IWindow* window) : IDevice(DeviceAPI::Direct3D, window)
{
	device = nullptr;
	context = nullptr;
	swapChain = nullptr;
	swapBackBuffer = nullptr;
	swapBackBufferSRV = nullptr;
}

DeviceDirect3D::~DeviceDirect3D()
{
	if(device) device->Release();
	if(context) context->Release();
	if(swapChain) swapChain->Release();
	if(swapBackBuffer) swapBackBuffer->Release();
}

bool DeviceDirect3D::create()
{
	HMODULE libD3D11 = LoadLibrary("d3d11.dll");

	if(!libD3D11)
	{
		Logger() << "Could not load d3d11.dll, you probably do not have DirectX 11 installed.";
		return false;
	}

	UINT createDeviceFlags = 0;
#if defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	DXGI_SWAP_CHAIN_DESC sd;
	
	ZeroMemory(&sd,sizeof(sd));
	
	sd.BufferCount = 1;	
	sd.BufferDesc.Width = (UINT)getWindow()->getWindowSettings().width;
	sd.BufferDesc.Height = (UINT)getWindow()->getWindowSettings().height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //_SRGB;
	sd.BufferDesc.RefreshRate.Numerator = 60;	
	sd.BufferDesc.RefreshRate.Denominator = 1;	
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.OutputWindow = static_cast<WindowWinAPI*>(getWindow())->getHandle();
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	
	HRESULT result = D3D11CreateDeviceAndSwapChain(0 , D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, featureLevels,
                        _countof(featureLevels), D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &context);
	FreeLibrary(libD3D11);

	if(result != S_OK)
	{
		LOGERROR(result, "D3D11CreateDeviceAndSwapChain");
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

	//Create the UAV for the swapbuffer
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = sd.BufferDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Buffer.FirstElement = 0; 
	uavDesc.Buffer.NumElements = sd.BufferDesc.Width * sd.BufferDesc.Height;

	ID3D11UnorderedAccessView* uavSwapBuffer;
	result = device->CreateUnorderedAccessView(swapBackBuffer, &uavDesc, &uavSwapBuffer);
	if(FAILED(result))
	{
		LOGERROR(result, "CreateUnorderedAccessView");
		return false;
	}

	return true;
}

void DeviceDirect3D::present()
{
	swapChain->Present(0, 0);
}

ICompute* DeviceDirect3D::createCompute()
{
	return new ComputeDirect3D(this);
}