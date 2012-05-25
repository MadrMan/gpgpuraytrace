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

	virtual bool create(const std::string& directory, const std::string& fileName, const std::string& main, const ThreadSize& ts, const std::vector<MacroType>& macros) = 0;

	virtual void run(unsigned int dispatchX, unsigned int dispatchY, unsigned int dispatchZ) = 0;

	virtual IShaderVariable* getVariable(const std::string& name) = 0;
	virtual IShaderArray* getArray(const std::string& name) = 0;
	virtual IShaderBuffer* getBuffer(const std::string& name) = 0;

	virtual bool swap() = 0;

	virtual void setTexture(int stage, ITexture* texture) = 0;

	const ThreadSize& getThreadSize() const
	{ return threadSize; }

protected:
	ICompute() { }

	ThreadSize threadSize;

private:

};

