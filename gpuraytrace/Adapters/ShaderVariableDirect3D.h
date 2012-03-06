#pragma once
#include "../Graphics/IShaderVariable.h"
#include "DeviceDirect3D.h"


class ConstantBufferD3D
{
public:
	int index;
	bool final;
	std::string name;
	int size;
	char* data;
	std::vector<IShaderVariable*> variables;
	bool dirty;
	ID3D11Buffer* gpuBuffer;
	DeviceDirect3D* device;

	virtual ~ConstantBufferD3D();
	ConstantBufferD3D(ID3D11Buffer** shaderBuffer, unsigned int shaderBufferIndex): shaderBuffer(shaderBuffer), shaderBufferIndex(shaderBufferIndex){};
	bool create(DeviceDirect3D* device);
	void finalize();
private:
	bool registerGpuBuffer(bool finalize);
	ID3D11Buffer** shaderBuffer;				
	unsigned int shaderBufferIndex;
};

class ShaderVariableDirect3D : public IShaderVariable
{
public:
	ShaderVariableDirect3D(const std::string name, int offset, int size, ConstantBufferD3D* buffer);
	virtual ~ShaderVariableDirect3D(){};
	virtual void write(void* data) override;
	virtual void finalizeBuffer() override;

	ConstantBufferD3D* getBuffer() const
	{ return buffer; }
	
	int getSizeInBytes()
	{ return sizeInBytes; }

private:
	int offset;
	int sizeInBytes;
	ConstantBufferD3D* buffer;
};


