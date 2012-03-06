#include "Common.h"
#include "ShaderVariableDirect3D.h"
#include "Logger.h"

ConstantBufferD3D::~ConstantBufferD3D(){
	for(auto it = variables.begin(); it != variables.end(); ++it)
	{
		ShaderVariableDirect3D* shaderVar = (ShaderVariableDirect3D*)*it;
		delete shaderVar;
	}
}

bool ConstantBufferD3D::registerGpuBuffer(bool finalize)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = size;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 
	bufferDesc.CPUAccessFlags = 0;
	
	HRESULT result;
	
	if(finalize)
	{
		final = true;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		D3D11_SUBRESOURCE_DATA initialData;
		ZeroMemory(&initialData, sizeof(initialData));
		initialData.pSysMem = data;
		result = device->getD3DDevice()->CreateBuffer(&bufferDesc, &initialData, &gpuBuffer);
	}else{
		result = device->getD3DDevice()->CreateBuffer(&bufferDesc, 0, &gpuBuffer);
	}
	shaderBuffer[index] = gpuBuffer; 

	if(result != S_OK) 
	{
		LOGERROR(result, "ID3D11Device::CreateBuffer");
		return false;
	}
	return true;
}

bool ConstantBufferD3D::create(DeviceDirect3D* device3D)
{
	if(!device)
		return false;
	
	final = false;	
	device = device3D;
	registerGpuBuffer(false);
}

void ConstantBufferD3D::finalize()
{
	registerGpuBuffer(true);
}


ShaderVariableDirect3D::ShaderVariableDirect3D(const std::string name, int offset, int size, ConstantBufferD3D* buffer) : IShaderVariable(name), offset(offset), sizeInBytes(size), buffer(buffer)
{
}

void ShaderVariableDirect3D::write(void* data)
{
	if(buffer->final){
		LOGFUNCERROR("Cannot write to final var");
		return;
	}
	
	memcpy(buffer->data + offset, data, sizeInBytes);
	buffer->dirty = true;
}

void ShaderVariableDirect3D::finalizeBuffer()
{
	buffer->finalize();
}
