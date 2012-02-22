#include "Common.h"
#include "ShaderVariableDirect3D.h"



ShaderVariableDirect3D::ShaderVariableDirect3D(const std::string name, int offset, int size, ConstantBufferD3D* buffer) : IShaderVariable(name), offset(offset), sizeInBytes(size), buffer(buffer)
{
}

void ShaderVariableDirect3D::write(void* data)
{
	memcpy(buffer->data + offset, data, sizeInBytes);
}