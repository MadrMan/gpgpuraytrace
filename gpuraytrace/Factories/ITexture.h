#pragma once

#include <string>

namespace TextureDimensions { enum T
{
	Texture1D,
	Texture2D,
	Texture3D
};}

class ITexture
{
public:
	virtual ~ITexture() { }

	virtual bool create(const std::string& path) = 0;

	virtual bool create(TextureDimensions::T dimensions, int width, int height, const void* data) = 0;

protected:
	ITexture() { }

private:

};