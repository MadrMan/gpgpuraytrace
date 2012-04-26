#pragma once

#include <string>

#include "IResource.h"

namespace TextureDimensions { enum T
{
	Texture1D,
	Texture2D,
	Texture3D
};}

namespace TextureFormat { enum T
{
	UNKNOWN,
	R8G8B8A8_SNORM,
	R8G8B8A8_UNORM,
	R8G8B8A8_UINT,
};}

namespace CPUAccess { enum T
{
	None,
	Read,
	Write,
	ReadWrite,
};}

namespace TextureBinding { enum T
{
	Texture,
	RenderTarget,
	Staging,
};}

class ITexture : public IResource
{
public:
	virtual ~ITexture() { }

	virtual bool create(const std::string& path) = 0;

	virtual bool create(TextureDimensions::T dimensions, TextureFormat::T format, int width, int height, const void* data, TextureBinding::T binding, CPUAccess::T cpuFlags) = 0;

protected:
	ITexture() { }

private:

};