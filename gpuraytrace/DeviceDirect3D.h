#pragma once

#include "IDevice.h"

#include <d3d11.h>
#include <D3DX11.h>

class DeviceDirect3D : public IDevice
{
public:
	DeviceDirect3D(IWindow* window);
	virtual ~DeviceDirect3D();

	virtual bool create() override;
	virtual void present() override;

	virtual ICompute* createCompute() override;

	ID3D11Device* getD3DDevice() const
	{ return device; }

	D3D_FEATURE_LEVEL getFeatureLevel() const
	{ return featureLevel; }

private:
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;
	ID3D11Texture2D* swapBackBuffer;
	ID3D11ShaderResourceView* swapBackBufferSRV;
	D3D_FEATURE_LEVEL featureLevel;
};

