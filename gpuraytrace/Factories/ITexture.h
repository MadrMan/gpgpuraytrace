#pragma once

#include <string>

class ITexture
{
public:
	virtual ~ITexture() { }

	virtual bool create(const std::string& path) = 0;

protected:
	ITexture() { }

private:

};