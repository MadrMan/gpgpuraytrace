#pragma once
#include "../Graphics/IShaderVariable.h"
#include "DeviceDirect3D.h"

#include <D3Dcompiler.h>

class ConstantBufferD3D : public IShaderBuffer
{
public:
	ConstantBufferD3D(DeviceDirect3D* device, const std::string& name, int size);
	virtual ~ConstantBufferD3D();

	virtual bool create(bool cpuWrite) override;
	void update(ID3D11DeviceContext* context);

	void change()
	{ dirty = true; }

	void write(unsigned int offset, void* data, unsigned int size);
	virtual void write(void* data) override;

	void addVariable(IShaderVariable* var)
	{ variables.push_back(var); }

	void* getData() const
	{ return data; }

	ID3D11Buffer* getGpuBuffer() const
	{ return gpuBuffer; }

private:
	DeviceDirect3D* device;
	void* data;
	unsigned int size;
	bool dirty;
	ID3D11Buffer* gpuBuffer;
};

class ShaderArrayDirect3D : public IShaderArray
{
public:
	ShaderArrayDirect3D(const std::string& name) : IShaderArray(name) { }

};

class UAVBufferD3D : public ShaderArrayDirect3D
{
public:
	UAVBufferD3D(DeviceDirect3D* device, const std::string& name, int stride);
	UAVBufferD3D(DeviceDirect3D* device, const std::string& name, ID3D11UnorderedAccessView* gpuView);
	virtual ~UAVBufferD3D();

	virtual bool create(unsigned int elements) override;
	virtual void write(void* data);
	virtual void* map() override;
	virtual void unmap() override;

	virtual ID3D11UnorderedAccessView* getView() const
	{ return gpuView; }

private:
	DeviceDirect3D* device;
	ID3D11Buffer* gpuBuffer;
	ID3D11Buffer* stagingBuffer;
	ID3D11UnorderedAccessView* gpuView;
	UINT stride;
};

class StructuredBufferD3D : public ShaderArrayDirect3D
{
public:
	StructuredBufferD3D(DeviceDirect3D* device, const std::string& name, int stride);
	virtual ~StructuredBufferD3D();

	virtual bool create(unsigned int elements);
	virtual void* map();
	virtual void unmap();
	virtual void write(void* data);

	virtual ID3D11ShaderResourceView* getView() const
	{ return gpuView; }

private:
	DeviceDirect3D* device;
	ID3D11Buffer* gpuBuffer;
	ID3D11ShaderResourceView* gpuView;
	UINT stride;
	int size;
};

class ShaderVariableDirect3D : public IShaderVariable
{
public:
	ShaderVariableDirect3D(const std::string& name, int offset, int size, ConstantBufferD3D* buffer);
	virtual ~ShaderVariableDirect3D() { };
	virtual void write(void* data) override;

	int getSizeInBytes()
	{ return sizeInBytes; }

private:
	int offset;
	int sizeInBytes;
};


