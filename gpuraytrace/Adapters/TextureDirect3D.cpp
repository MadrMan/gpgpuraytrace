#include <Common.h>
#include "TextureDirect3D.h"

#include "DeviceDirect3D.h"

TextureDirect3D::TextureDirect3D(DeviceDirect3D* device) : device(device)
{
	resource = nullptr;
	view = nullptr;
}

TextureDirect3D::~TextureDirect3D()
{

}

bool TextureDirect3D::create(const std::string& path)
{
	HRESULT result = D3DX11CreateTextureFromFile(device->getD3DDevice(), path.c_str(), nullptr, nullptr, &resource, nullptr);
	if(FAILED(result))
	{
		return false;
	}

	result = device->getD3DDevice()->CreateShaderResourceView(resource, nullptr, &view);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}