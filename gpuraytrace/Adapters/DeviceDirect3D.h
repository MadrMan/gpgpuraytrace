#pragma once

#include "../Factories/IDevice.h"
#include "../Factories/IRecorder.h"

#include <d3d11.h>

class TextureDirect3D;
class RecorderWinAPI;
class DeviceDirect3D : public IDevice
{
public:
	DeviceDirect3D(IWindow* window);
	virtual ~DeviceDirect3D();

	virtual bool create() override;
	virtual void present() override;
	virtual void flush() override;

	virtual ICompute* createCompute() override;

	virtual ITexture* createTexture() override;

	void setRecorder(RecorderWinAPI* recorder);

	ID3D11Device* getD3DDevice() const
	{ return device; }

	D3D_FEATURE_LEVEL getFeatureLevel() const
	{ return featureLevel; }

	ID3D11DeviceContext* getImmediate() const
	{ return context; }

	ID3D11UnorderedAccessView* getSwapUAV() const
	{ return uavSwapBuffer; }

private:
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;
	ID3D11Texture2D* swapBackBuffer;
	ID3D11ShaderResourceView* swapBackBufferSRV;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11UnorderedAccessView* uavSwapBuffer;

	TextureDirect3D* swapStaging;
	RecorderWinAPI* recorder;
};

