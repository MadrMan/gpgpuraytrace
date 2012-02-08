#pragma once

//! Interfaces inherited by all devices
class ICompute
{
public:
	//! Destructor
	virtual ~ICompute() { }

	virtual bool create(const std::string& fileName, const std::string& main) = 0;

	virtual void run() = 0;

protected:
	ICompute() { }

};

