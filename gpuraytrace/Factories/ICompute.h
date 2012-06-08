#pragma once

#include "../Graphics/IShaderVariable.h"
#include "../Common/Settings.h"

class ITexture;

struct ThreadSize
{
	int x, y, z;
};

typedef std::pair<std::string, std::string> MacroType;

//! Interfaces inherited by all devices
class ICompute
{
public:
	//! Destructor
	virtual ~ICompute() { }

	//! Create a new compute shader
	//! \param directory Directory in which the shader resides
	//! \param fileName Filename of the shader
	//! \param main Entrypoint for the shader
	//! \param ts Thread size to compile the shader with
	//! \param macros Macros to supply to the shader upon compilation
	virtual bool create(const std::string& directory, const std::string& fileName, const std::string& main, const ThreadSize& ts, const std::vector<MacroType>& macros) = 0;

	//! Run the shader with the specified dispatch dimensions
	virtual void run(unsigned int dispatchX, unsigned int dispatchY, unsigned int dispatchZ) = 0;

	//! Get a variable from the shader
	virtual IShaderVariable* getVariable(const std::string& name) = 0;

	//! Get an array from the shader
	virtual IShaderArray* getArray(const std::string& name) = 0;

	//! Get a buffer from the shader
	virtual IShaderBuffer* getBuffer(const std::string& name) = 0;

	//! Call this when a new shader has been created and the current one needs to be replaced
	virtual bool swap() = 0;

	//! Set a texture on the shader
	virtual void setTexture(int stage, ITexture* texture) = 0;

	//! Get the thread size with which the shader was compiled
	const ThreadSize& getThreadSize() const
	{ return threadSize; }

protected:
	ICompute() { }

	ThreadSize threadSize;

private:

};

