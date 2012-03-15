#pragma once

#include "../Graphics/IShaderVariable.h"

class ITexture;

//! Interfaces inherited by all devices
class ICompute
{
public:
	//! Destructor
	virtual ~ICompute() { }

	virtual bool create(const std::string& directory, const std::string& fileName, const std::string& main) = 0;

	virtual void run(unsigned int dispatchX, unsigned int dispatchY, unsigned int dispatchZ) = 0;

	virtual IShaderVariable* getVariable(const std::string& name) = 0;
	virtual IShaderArray* getArray(const std::string& name) = 0;
	virtual IShaderBuffer* getBuffer(const std::string& name) = 0;

	virtual bool swap() = 0;

	virtual void setTexture(int stage, ITexture* texture) = 0;

protected:
	ICompute() { }
};

