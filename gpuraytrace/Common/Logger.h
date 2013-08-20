#pragma once

#include <ostream>

#define LOGERROR(x, f) Logger() << __FUNCTION__ << " failed at " << f << " = 0x" << std::hex << x << std::dec;
#define LOGFUNCERROR(x) Logger() << __FUNCTION__ << " failed: " << x;

//! Buffer which logs all the text from the Logger to the outputs
class LogBuffer : public std::streambuf
{
private:
	virtual int overflow(int c) override;
	virtual int sync() override;

	std::string buffer;
};

//! Stream used to forward data from the Logger to the LogBuffer
class LogStream : public std::ostream
{
public:
	//! Constructor
	//! \param buffer Buffer to forward to
	LogStream(LogBuffer& buffer);
};

//! Class to log text with
//! \code
//! Logger() << "I'm logging text and a number " << 10 << " here";
//! \endcode
class Logger
{
public:
	//! Constructor
	Logger();

	//! Destructor
	virtual ~Logger();

	//! Operator which accepts all streamed input
	//! \param x Streamable data to log
	template<typename T>
	Logger& operator<<(const T x)
	{
		*stream << x;
		return *this;
	}

private:
	LogStream* stream;
	Logger(Logger const& ) : stream(nullptr){};
	Logger& operator=(Logger const&);
    
};