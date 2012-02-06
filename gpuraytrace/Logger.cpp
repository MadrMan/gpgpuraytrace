#include "Common.h"
#include "Logger.h"

#include <iostream>
#include <string>

LogBuffer logBuffer;
LogStream logStream(logBuffer);

int LogBuffer::overflow(int c)
{
	buffer += (char)c;
	return c;
}

int LogBuffer::sync()
{
	std::cout << buffer << std::endl;

	buffer.clear();
	return 0;
}

LogStream::LogStream(LogBuffer& buffer) : std::ostream(&buffer)
{
}

Logger::Logger() : stream(logStream)
{
}

Logger::~Logger()
{
	stream.flush();
}
