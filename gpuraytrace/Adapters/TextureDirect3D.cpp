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
	if(resource) resource->Release();
	if(view) view->Release();
}

bool TextureDirect3D::create(const std::string& path)
{
	UNREFERENCED_PARAMETER(path);

	/*device->getD3DDevice()->CreateTexture2D(
	HRESULT result = D3DX11CreateTextureFromFile(device->getD3DDevice(), path.c_str(), nullptr, nullptr, &resource, nullptr);
	if(FAILED(result))
	{
		return false;
	}

	result = device->getD3DDevice()->CreateShaderResourceView(resource, nullptr, &view);
	if(FAILED(result))
	{
		return false;
	}*/

	return false;
}

bool TextureDirect3D::create(TextureDimensions::T dimensions, int width, int height, const void* dataptr)
{
	HRESULT result = S_OK;

	switch(dimensions)
	{
		case TextureDimensions::Texture1D:
		{
			D3D11_TEXTURE1D_DESC desc = {0};
			desc.Width = width;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			//desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			desc.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.CPUAccessFlags = 0;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA data = {0};
			data.pSysMem = dataptr;
			data.SysMemPitch = width * sizeof(char) * 4;
			data.SysMemSlicePitch = data.SysMemPitch * height;

			ID3D11Texture1D* texture;
			result = device->getD3DDevice()->CreateTexture1D(&desc, &data, &texture);
			resource = texture;
		} break;
		case TextureDimensions::Texture2D:
		{
			D3D11_TEXTURE2D_DESC desc = {0};
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			//desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_IMMUTABLE;
			desc.CPUAccessFlags = 0;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA data = {0};
			data.pSysMem = dataptr;
			data.SysMemPitch = width * sizeof(char) * 4;
			data.SysMemSlicePitch = data.SysMemPitch * height;

			ID3D11Texture2D* texture;
			result = device->getD3DDevice()->CreateTexture2D(&desc, &data, &texture);
			resource = texture;
		} break;
		default:
			result = E_FAIL;
	}

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