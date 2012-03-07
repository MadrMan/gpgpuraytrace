#pragma once
#include "../Graphics/IShaderVariable.h"
#include "DeviceDirect3D.h"

#include <D3Dcompiler.h>

class BufferD3D
{
public:
	std::string name;
	int size;
	
	DeviceDirect3D* device;
	std::vector<IShaderVariable*> variables;
	
	virtual bool create() = 0;

	//! Call this to tell the buffer the data has been modified
	virtual void change() = 0;
	virtual void* map(unsigned int offset, unsigned int size) = 0;
	virtual void unmap() = 0;

	virtual void write(unsigned int offset, void* data, unsigned int size) = 0;
protected:
	BufferD3D(DeviceDirect3D* device, D3D11_SHADER_BUFFER_DESC& desc);
	virtual ~BufferD3D();
	
private:

};

class ConstantBufferD3D : public BufferD3D
{
public:
	ConstantBufferD3D(DeviceDirect3D* device, D3D11_SHADER_BUFFER_DESC& desc);
	virtual ~ConstantBufferD3D();

	void finalize();
	virtual bool create() override;
	virtual void change() override;
	virtual void write(unsigned int offset, void* data, unsigned int size) override;
	virtual void* map(unsigned int offset, unsigned int size) override;
	virtual void unmap() override;

	char* data;
	bool final;
	bool dirty;
	ID3D11Buffer* gpuBuffer;
};

class UAVBufferD3D : public BufferD3D
{
public:
	UAVBufferD3D(DeviceDirect3D* device, D3D11_SHADER_BUFFER_DESC& desc, UINT stride);
	virtual ~UAVBufferD3D();

	virtual bool create() override;
	virtual void change() override;
	virtual void write(unsigned int offset, void* data, unsigned int size) override;
	virtual void* map(unsigned int offset, unsigned int size) override;
	virtual void unmap() override;

	ID3D11Buffer* gpuBuffer;
	ID3D11Buffer* stagingBuffer;
	ID3D11UnorderedAccessView* gpuView;
	UINT stride;
};

class ShaderVariableDirect3D : public IShaderVariable
{
public:
	ShaderVariableDirect3D(const std::string name, int offset, int size, BufferD3D* buffer);
	virtual ~ShaderVariableDirect3D(){};
	virtual void write(void* data) override;
	virtual void finalize() override;
	virtual void* map() override;
	virtual void unmap() override;

	BufferD3D* getBuffer() const
	{ return buffer; }
	
	int getSizeInBytes()
	{ return sizeInBytes; }

private:
	int offset;
	int sizeInBytes;
	BufferD3D* buffer;
};


