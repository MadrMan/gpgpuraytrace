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

//! Interface used by all textures
class ITexture : public IResource
{
public:
	//! Destructor
	virtual ~ITexture() { }

	//! Create a texture from disk
	//! \param path Path to the filename used to load the texture from
	virtual bool create(const std::string& path) = 0;

	//! Create a texture from memory
	//! \param dimensions Dimensions
	//! \param format Format
	//! \param width Width of the texture
	//! \param height Height of the texture - Ignored in 1D
	//! \param data Data from which to create the texture
	//! \param binding Specifies what usage the texture will have
	//! \param cpuFlags Flags that specify the allowed cpu access for the created texture
	virtual bool create(TextureDimensions::T dimensions, TextureFormat::T format, int width, int height, const void* data, TextureBinding::T binding, CPUAccess::T cpuFlags) = 0;

protected:
	ITexture() { }

private:

};