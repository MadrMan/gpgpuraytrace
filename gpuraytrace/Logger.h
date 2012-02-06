#pragma once

#include <ostream>

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
		stream << x;
		return *this;
	}

private:
	LogStream& stream;
};