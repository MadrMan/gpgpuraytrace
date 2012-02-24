#pragma once

class IShaderVariable;

//! Interfaces inherited by all devices
class ICompute
{
public:
	//! Destructor
	virtual ~ICompute() { }

	virtual bool create(const std::string& directory, const std::string& fileName, const std::string& main) = 0;

	virtual void run() = 0;

	virtual IShaderVariable* getVariable(const std::string& name) = 0;

	virtual bool swap() = 0;

protected:
	ICompute() { }
};

