#pragma once

#include <ostream>

#define LOGERROR(x, f) Logger() << __FUNCTION__ << " failed at " << (f) << " with 0x" << std::hex << (x) << std::dec;

class LogBuffer : public std::streambuf
{
	virtual int overflow(int c) override;
	virtual int sync() override;
private:
	std::string buffer;
};

class LogStream : public std::ostream
{
public:
	LogStream(LogBuffer& buffer);
};

class Logger
{
public:
	Logger();
	virtual ~Logger();

	template<typename T>
	Logger& operator<<(const T x)
	{
		*stream << x;
		return *this;
	}

private:
	LogStream* stream;
	Logger(Logger const& ) : stream(nullptr){};
	Logger& operator=(Logger const&){};          
    
};