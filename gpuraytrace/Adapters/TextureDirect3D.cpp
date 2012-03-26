#include <Common.h>
#include "TextureDirect3D.h"

#include "../Common/Logger.h"
#include "DeviceDirect3D.h"

#include <D3DX11.h>

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

bool TextureDirect3D::create(TextureDimensions::T dimensions, TextureFormat::T format, int width, int height, const void* dataptr, TextureBinding::T binding, CPUAccess::T cpuFlags)
{
	if((binding & TextureBinding::Staging) && dataptr)
	{
		LOGFUNCERROR("TextureBinding::Staging is incompatible with predefined data");
		return false;
	}

	DXGI_FORMAT formatDxgi = DXGI_FORMAT_FORCE_UINT;
	switch(format)
	{
	case TextureFormat::UNKNOWN:
		formatDxgi = DXGI_FORMAT_UNKNOWN;
		break;
	case TextureFormat::R8G8B8A8_UINT:
		formatDxgi = DXGI_FORMAT_R8G8B8A8_UINT;
		break;
	case TextureFormat::R8G8B8A8_UNORM:
		formatDxgi = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case TextureFormat::R8G8B8A8_SNORM:
		formatDxgi = DXGI_FORMAT_R8G8B8A8_SNORM;
		break;
	default:
		LOGFUNCERROR("Unsupported texture format");
		return false;
		break;
	}

	UINT cpuFlagsDx = 0;
	if(cpuFlags == CPUAccess::Read) cpuFlagsDx = D3D11_CPU_ACCESS_READ;
	if(cpuFlags == CPUAccess::Write) cpuFlagsDx = D3D11_CPU_ACCESS_WRITE;
	if(cpuFlags == CPUAccess::ReadWrite) cpuFlagsDx = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

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
			desc.Format = formatDxgi;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = cpuFlagsDx;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA data = {0};
			if(dataptr)
			{
				desc.Usage = D3D11_USAGE_IMMUTABLE;
				data.pSysMem = dataptr;
				data.SysMemPitch = width * sizeof(char) * 4;
				data.SysMemSlicePitch = data.SysMemPitch * height;
			}

			ID3D11Texture1D* texture;
			result = device->getD3DDevice()->CreateTexture1D(&desc, dataptr ? &data : nullptr, &texture);
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
			desc.Format = formatDxgi;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = cpuFlagsDx;
			
			desc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA data = {0};
			if(dataptr)
			{
				desc.Usage = D3D11_USAGE_IMMUTABLE;
				data.pSysMem = dataptr;
				data.SysMemPitch = width * sizeof(char) * 4;
				data.SysMemSlicePitch = data.SysMemPitch * height;
			}

			if(binding & TextureBinding::Staging)
			{
				desc.Usage = D3D11_USAGE_STAGING;
			}  else {
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			}

			if(binding & TextureBinding::RenderTarget)
			{
				desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			}

			ID3D11Texture2D* texture;
			result = device->getD3DDevice()->CreateTexture2D(&desc, dataptr ? &data : nullptr, &texture);
			resource = texture;
		} break;
		default:
			result = E_FAIL;
	}

	if(FAILED(result))
	{
		LOGERROR(result, "CreateTexture");
		return false;
	}

	if(binding != TextureBinding::Staging)
	{
		result = device->getD3DDevice()->CreateShaderResourceView(resource, nullptr, &view);
		if(FAILED(result))
		{
			return false;
		}
	}

	return true;
}