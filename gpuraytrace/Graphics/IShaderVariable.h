#pragma once

#include <string>
#include "../Factories/IResource.h"

//! Interface which is the same for every variable which can be obtained from a shader
class IShaderVariable : public IResource
{
public:
	//! Destructor
	virtual ~IShaderVariable() { }

	//! Get the name of the variable
	const std::string& getName() const
	{ return name; }

	//! True if the variable can be written to
	bool isWritable() const
	{ return writable; }

	//! Get the parent variable (for when this variable is in a buffer or such)
	IShaderVariable* getParent() const
	{ return parent; }

	//! Write new data to this variable
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

//! Interface which is used for every array in shaders
class IShaderArray : public IShaderVariable
{
public:
	//! Destructor
	virtual ~IShaderArray() { }

	//! Create an instance of the array
	//! \param elements Amount of elements the array should contain
	virtual bool create(unsigned int elements) = 0;

	//! Map the array for reading or writing
	virtual void* map() = 0;

	//! Unmap the array and update it
	virtual void unmap()= 0;

protected:
	IShaderArray(const std::string& name) : IShaderVariable(name, nullptr) { }
	
};

//! Interface which is used for every buffer in shaders
class IShaderBuffer : public IShaderVariable
{
public:
	//! Destructor
	virtual ~IShaderBuffer() { }

	//! Create an instance of the buffer
	//! \param cpuWrite Whether or not the buffer should be writable by the cpu
	virtual bool create(bool cpuWrite) = 0;

	//! Get all the variables in this buffer
	const std::vector<IShaderVariable*> getVariables() const
	{ return variables; }

protected:
	IShaderBuffer(const std::string& name) : IShaderVariable(name, nullptr) { }
	
	std::vector<IShaderVariable*> variables;
};