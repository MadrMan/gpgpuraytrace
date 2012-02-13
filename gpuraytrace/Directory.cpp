#include <Common.h>
#include "Directory.h"
#include "Logger.h"

#include "./Adapters/DirectoryWinAPI.h"

Directory* Directory::directory;
Directory* Directory::get()
{
	if(directory) return directory;

#if defined(_WIN32)
	directory = new DirectoryWinAPI();
#else
	Logger() << "Could not create directory handler";
#endif

	return directory;
}