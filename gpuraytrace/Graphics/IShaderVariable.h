#pragma once

#include <string>
#include "../Factories/IResource.h"

class IShaderVariable : public IResource
{
public:
	virtual ~IShaderVariable() { }

	const std::string& getName() const
	{ return name; }

	bool isWritable() const
	{ return writable; }

	IShaderVariable* getParent() const
	{ return parent; }

	virtual void write(void* data) = 0;

protected:
	IShaderVariable(const std::string& name, IShaderVariable* parent) : 
		name(name), writable(false), parent(parent) { }
		
	void setWritable(bool writable)
	{ this->writable = writable; }

private:
	IShaderVariable* parent;
	std::string name;
	bool writable;
};

class IShaderArray : public IShaderVariable
{
public:
	virtual ~IShaderArray() { }

	virtual bool create(unsigned int elements) = 0;
	virtual void* map() = 0;
	virtual void unmap()= 0;

protected:
	IShaderArray(const std::string& name) : IShaderVariable(name, nullptr) { }
	
};

class IShaderBuffer : public IShaderVariable
{
public:
	virtual ~IShaderBuffer() { }

	virtual bool create(bool cpuWrite) = 0;

	const std::vector<IShaderVariable*> getVariables() const
	{ return variables; }

protected:
	IShaderBuffer(const std::string& name) : IShaderVariable(name, nullptr) { }
	
	std::vector<IShaderVariable*> variables;
};