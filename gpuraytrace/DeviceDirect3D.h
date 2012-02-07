#pragma once

#include "IDevice.h"

#include <d3d11.h>

class DeviceDirect3D : public IDevice
{
public:
	DeviceDirect3D(IWindow* window);
	virtual ~DeviceDirect3D();

	virtual bool create() override;
	virtual void present() override;


private:
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;
	ID3D11Texture2D* swapBackBuffer;
};

