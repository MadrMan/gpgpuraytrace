#include "Common.h"
#include "ShaderVariableDirect3D.h"


ConstantBufferD3D::~ConstantBufferD3D(){
	for(auto it = variables.begin(); it != variables.end(); ++it)
	{
		ShaderVariableDirect3D* shaderVar = (ShaderVariableDirect3D*)*it;
		delete shaderVar;
	}
}


ShaderVariableDirect3D::ShaderVariableDirect3D(const std::string name, int offset, int size, ConstantBufferD3D* buffer) : IShaderVariable(name), offset(offset), sizeInBytes(size), buffer(buffer)
{
}

void ShaderVariableDirect3D::write(void* data)
{
	memcpy(buffer->data + offset, data, sizeInBytes);
}