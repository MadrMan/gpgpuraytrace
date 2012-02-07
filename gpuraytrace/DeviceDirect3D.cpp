#include "Common.h"
#include "DeviceDirect3D.h"

#include "WindowWinAPI.h"
#include "Logger.h"

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
#if defined(FYX_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevel;
 

	const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	DXGI_SWAP_CHAIN_DESC sd;
	
	ZeroMemory(&sd,sizeof(sd));
	
	sd.BufferCount = 1;	
	sd.BufferDesc.Width = (UINT)getWindow()->getWindowSettings().width;
	sd.BufferDesc.Height = (UINT)getWindow()->getWindowSettings().height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //_SRGB;
	sd.BufferDesc.RefreshRate.Numerator = 60;	
	sd.BufferDesc.RefreshRate.Denominator = 1;	
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = static_cast<WindowWinAPI*>(getWindow())->getHandle();
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = FALSE;
	
	HRESULT result = D3D11CreateDeviceAndSwapChain(0 , D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, featureLevels,
                        _countof(featureLevels), D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &context);
	FreeLibrary(libD3D11);

	if(result != S_OK)
	{
		LOGERROR(result, "D3D11CreateDeviceAndSwapChain");
		return false;
	}


	//Get the buffer from the swapchain
	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&swapBackBuffer);
	if(result != S_OK)
    {
		LOGERROR(result, "IDXGISwapChain::GetBuffer");
        return false;
    }

	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;	
	result = device->CreateShaderResourceView(swapBackBuffer, 0, &swapBackBufferSRV);

	if(result != S_OK){
		LOGERROR(result, "ID3D11Device::CreateShaderResourceView");
		return false;
	}

	const std::string fileName("shader/csmain.hlsl");
	std::ifstream file(fileName);
	file.seekg(0, std::ios_base::end);
	unsigned int pos = (unsigned int)file.tellg();
	char* fileData;
	fileData = new char[pos + 1];
	file.seekg(0, std::ios_base::beg);
	file.read(fileData, pos);
	file.close();
	fileData[pos] = 0;

	UINT shaderFlags;
#if defined(_DEBUG)
	shaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_OPTIMIZATION_LEVEL0;
#else
	shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
	
	ID3D10Blob* shaderBlob;
	ID3D10Blob* errorBlob;
	result = D3DCompile(fileData, pos + 1, fileName.c_str(), nullptr, ((ID3DInclude*)(UINT_PTR)1), "CSMain", "cs_5_0", 0, shaderFlags, &shaderBlob, &errorBlob);
	if(result != S_OK)
	{
		errorBlob->Release();
		return false;
	}
	
	ID3D11ComputeShader* computeShader;
	result = device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &computeShader);
	if(result != S_OK)
	{
		return false;
	}

	return true;
}

void DeviceDirect3D::present()
{
	
		

}