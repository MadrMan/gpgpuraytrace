#pragma once

#include "../Factories/ITexture.h"

class DeviceDirect3D;
class TextureDirect3D : public ITexture
{
public:
	TextureDirect3D(DeviceDirect3D* device);
	virtual ~TextureDirect3D();

	virtual bool create(const std::string& path) override;

	virtual bool create(TextureDimensions::T dimensions, TextureFormat::T format, int width, int height, const void* data, TextureBinding::T binding, CPUAccess::T cpuFlags) override;

	ID3D11Resource* getResource() const
	{ return resource; }

	ID3D11ShaderResourceView* getView() const
	{ return view; }

private:
	DeviceDirect3D* device;
	ID3D11Resource* resource;
	ID3D11ShaderResourceView* view;
};