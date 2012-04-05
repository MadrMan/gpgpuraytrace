#include <Common.h>
#include "./ShaderVariableDirect3D.h"

#include "./Common/Logger.h"

const int BUFFER_PADSIZE = 16;

ConstantBufferD3D::ConstantBufferD3D(DeviceDirect3D* device, const std::string name, int size) : IShaderBuffer(name), device(device), size(size)
{
	data = new char[size];
	ZeroMemory(data, size);

	gpuBuffer = nullptr;
}

ConstantBufferD3D::~ConstantBufferD3D()
{
	if(gpuBuffer) gpuBuffer->Release();
}

bool ConstantBufferD3D::create(bool cpuWrite)
{
	setWritable(cpuWrite);

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = (size % BUFFER_PADSIZE) ? size + BUFFER_PADSIZE - size % BUFFER_PADSIZE : size;

	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	if(cpuWrite)
	{
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	} else {
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
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

void ConstantBufferD3D::update(ID3D11DeviceContext* context)
{
	if(dirty)
	{
		dirty = false;
		context->UpdateSubresource(gpuBuffer, 0, 0, data, size, 0);
	}
}

void ConstantBufferD3D::write(unsigned int offset, void* src, unsigned int writeSize)
{
	if(!isWritable()) 
	{
		LOGFUNCERROR("Cannot write to unwritable cbuffer");
		return;
	}

	if(writeSize > size)
	{
		LOGFUNCERROR("Data size to write is larger than buffer size (" << writeSize << " > " << size << ")");
		return;
	}

	memcpy((char*)data + offset, src, writeSize);

	change();
}

void ConstantBufferD3D::write(void* data)
{
	write(0, data, size);
}

UAVBufferD3D::UAVBufferD3D(DeviceDirect3D* device, const std::string name, int stride) : IShaderArray(name), stride(stride), device(device)
{
	gpuBuffer = nullptr;
	stagingBuffer = nullptr;
	gpuView = nullptr;
}

UAVBufferD3D::UAVBufferD3D(DeviceDirect3D* device, const std::string name, ID3D11UnorderedAccessView* gpuView) : IShaderArray(name), gpuView(gpuView), device(device)
{
	gpuView->AddRef();

	stride = 0;
	gpuBuffer = nullptr;
	stagingBuffer = nullptr;
}

UAVBufferD3D::~UAVBufferD3D()
{
	if(gpuBuffer) gpuBuffer->Release();
	if(stagingBuffer) stagingBuffer->Release();
	if(gpuView) gpuView->Release();
}

bool UAVBufferD3D::create(bool cpuWrite, unsigned int elements)
{
	setWritable(cpuWrite);

	if(gpuView) 
	{
		LOGFUNCERROR("SRV already created");
		return false;
	}

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = elements * stride; //(size % BUFFER_PADSIZE) ? size + BUFFER_PADSIZE - size % BUFFER_PADSIZE : size;

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
	uavDesc.Buffer.NumElements = elements;
	result = device->getD3DDevice()->CreateUnorderedAccessView(gpuBuffer, &uavDesc, &gpuView);
	if(FAILED(result)) 
	{
		LOGERROR(result, "ID3D11Device::CreateUnorderedAccessView");
		return false;
	}

	return true;
}

void UAVBufferD3D::write(void* data)
{
	LOGFUNCERROR("Write to UAV not supported");
}

void* UAVBufferD3D::map()
{
	if(!gpuBuffer) 
	{
		LOGFUNCERROR("No SRV created");
		return nullptr;
	}

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

ShaderVariableDirect3D::ShaderVariableDirect3D(const std::string& name, int offset, int size, ConstantBufferD3D* buffer) : IShaderVariable(name, buffer), offset(offset), sizeInBytes(size)
{
}

void ShaderVariableDirect3D::write(void* data)
{
	static_cast<ConstantBufferD3D*>(getParent())->write(offset, data, sizeInBytes);
}