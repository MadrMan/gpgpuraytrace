#include "Common.h"
#include "ShaderVariableDirect3D.h"
#include "Logger.h"

const int BUFFER_PADSIZE = 16;

BufferD3D::BufferD3D(DeviceDirect3D* device, D3D11_SHADER_BUFFER_DESC& desc) : device(device)
{
	size = desc.Size;
	name = desc.Name;
}

BufferD3D::~BufferD3D()
{
	for(auto it = variables.begin(); it != variables.end(); ++it)
	{
		ShaderVariableDirect3D* shaderVar = (ShaderVariableDirect3D*)*it;
		delete shaderVar;
	}
}

ConstantBufferD3D::ConstantBufferD3D(DeviceDirect3D* device, D3D11_SHADER_BUFFER_DESC& desc) : BufferD3D(device, desc)
{
	data = new char[size];
	ZeroMemory(data, size);

	final = false;
	gpuBuffer = nullptr;
}

ConstantBufferD3D::~ConstantBufferD3D()
{
	if(gpuBuffer) gpuBuffer->Release();
}

bool ConstantBufferD3D::create()
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = (size % BUFFER_PADSIZE) ? size + BUFFER_PADSIZE - size % BUFFER_PADSIZE : size;

	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	if(final)
	{
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	} else {
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	}

	D3D11_SUBRESOURCE_DATA initialData;
	ZeroMemory(&initialData, sizeof(initialData));
	initialData.pSysMem = data;

	HRESULT result = device->getD3DDevice()->CreateBuffer(&bufferDesc, &initialData, &gpuBuffer);
	if(FAILED(result)) 
	{
		LOGERROR(result, "ID3D11Device::CreateBuffer");
		return false;
	}

	return true;
}

void ConstantBufferD3D::change()
{
	dirty = true;
}

void ConstantBufferD3D::finalize()
{
	final = true;
	create();
}

void ConstantBufferD3D::write(unsigned int offset, void* src, unsigned int size)
{
	if(final) 
	{
		LOGFUNCERROR("Cannot write to finalized cbuffer");
		return;
	}

	memcpy((char*)data + offset, src, size);

	change();
}

void* ConstantBufferD3D::map(unsigned int offset, unsigned int size)
{
	return nullptr;

	/*D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT result = device->getImmediate()->Map(stagingBuffer, 0, D3D11_MAP_READ_WRITE, 0, &mapped);
	if(FAILED(result)) return nullptr;
	return nullptr;*/
}

void ConstantBufferD3D::unmap()
{
	//device->getImmediate()->Unmap(stagingBuffer, 0);
}

UAVBufferD3D::UAVBufferD3D(DeviceDirect3D* device, D3D11_SHADER_BUFFER_DESC& desc, UINT stride) : BufferD3D(device, desc), stride(stride)
{
	gpuBuffer = nullptr;
	stagingBuffer = nullptr;
	gpuView = nullptr;
}

UAVBufferD3D::~UAVBufferD3D()
{
	if(gpuBuffer) gpuBuffer->Release();
	if(stagingBuffer) stagingBuffer->Release();
	if(gpuView) gpuView->Release();
}

void UAVBufferD3D::change()
{

}

bool UAVBufferD3D::create()
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = (size % BUFFER_PADSIZE) ? size + BUFFER_PADSIZE - size % BUFFER_PADSIZE : size;

	if(bufferDesc.ByteWidth > D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * sizeof(float))
	{
		LOGFUNCERROR("Size of buffer exceeds maximum size");
		return false;
	}

	bufferDesc.BindFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = stride;

	HRESULT result = device->getD3DDevice()->CreateBuffer(&bufferDesc, nullptr, &stagingBuffer);
	if(FAILED(result)) 
	{
		LOGERROR(result, "ID3D11Device::CreateBuffer Staging");
		return false;
	}

	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.CPUAccessFlags = 0;
	
	result = device->getD3DDevice()->CreateBuffer(&bufferDesc, nullptr, &gpuBuffer);
	if(FAILED(result)) 
	{
		LOGERROR(result, "ID3D11Device::CreateBuffer Default");
		return false;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0; 
	uavDesc.Buffer.NumElements = size / stride;
	result = device->getD3DDevice()->CreateUnorderedAccessView(gpuBuffer, &uavDesc, &gpuView);
	if(FAILED(result)) 
	{
		LOGERROR(result, "ID3D11Device::CreateUnorderedAccessView");
		return false;
	}

	return true;
}

void UAVBufferD3D::write(unsigned int offset, void* data, unsigned int size)
{
}

void* UAVBufferD3D::map(unsigned int offset, unsigned int size)
{
	ID3D11DeviceContext* context = device->getImmediate();
	context->CopyResource(stagingBuffer, gpuBuffer);
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT result = context->Map(stagingBuffer, 0, D3D11_MAP_READ_WRITE, 0, &mapped);
	if(FAILED(result)) return nullptr;
	return mapped.pData;
}

void UAVBufferD3D::unmap()
{
	ID3D11DeviceContext* context = device->getImmediate();
	context->Unmap(stagingBuffer, 0);
	context->CopyResource(gpuBuffer, stagingBuffer);
}

ShaderVariableDirect3D::ShaderVariableDirect3D(const std::string name, int offset, int size, BufferD3D* buffer) : IShaderVariable(name), offset(offset), sizeInBytes(size), buffer(buffer)
{
}

void ShaderVariableDirect3D::write(void* data)
{
	buffer->write(offset, data, sizeInBytes);
}

void ShaderVariableDirect3D::finalize()
{
	ConstantBufferD3D* cbuffer = static_cast<ConstantBufferD3D*>(buffer);
	cbuffer->finalize();
}

void* ShaderVariableDirect3D::map()
{
	return buffer->map(offset, sizeInBytes);
}

void ShaderVariableDirect3D::unmap()
{
	return buffer->unmap();
}