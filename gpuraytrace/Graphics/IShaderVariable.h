#pragma once

#include <string>

class IShaderVariable
{
public:
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
		virtual ~IShaderVariable() { }

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
	virtual bool create(bool cpuWrite, unsigned int elements) = 0;
	virtual void* map() = 0;
	virtual void unmap()= 0;

protected:
	IShaderArray(const std::string& name) : IShaderVariable(name, nullptr) { }
	virtual ~IShaderArray() { }
};

class IShaderBuffer : public IShaderVariable
{
public:
	virtual bool create(bool cpuWrite) = 0;

	const std::vector<IShaderVariable*> getVariables() const
	{ return variables; }

protected:
	IShaderBuffer(const std::string& name) : IShaderVariable(name, nullptr) { }
	virtual ~IShaderBuffer() { }

	std::vector<IShaderVariable*> variables;
};