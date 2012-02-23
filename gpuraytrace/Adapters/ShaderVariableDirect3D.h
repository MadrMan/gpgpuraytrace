#pragma once
#include "../Graphics/IShaderVariable.h"

class ConstantBufferD3D
{
public:
	int index;
	std::string name;
	int size;
	char* data;
	std::vector<IShaderVariable*> variables;
	bool dirty;
	ID3D11Buffer* gpuBuffer;
	
	virtual ~ConstantBufferD3D();
};

class ShaderVariableDirect3D : public IShaderVariable
{
public:
	ShaderVariableDirect3D(const std::string name, int offset, int size, ConstantBufferD3D* buffer);
	virtual ~ShaderVariableDirect3D(){};
	virtual void write(void* data) override;

	ConstantBufferD3D* getBuffer() const
	{ return buffer; }

private:
	int offset;
	int sizeInBytes;
	ConstantBufferD3D* buffer;
};


