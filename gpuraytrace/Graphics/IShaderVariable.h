#pragma once

#include <string>

class IShaderVariable
{
public:
	IShaderVariable(const std::string name) : name(name) { }
	virtual ~IShaderVariable() { };

	virtual void write(void* data) = 0;
	virtual void finalizeBuffer() = 0;

	const std::string& getName() const
	{ return name; }

private:
	std::string name;
};