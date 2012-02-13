#pragma once

#include "Directory.h"

#include <map>

struct DirWinAPIWait
{
	IDirectoryCallbackBase* cb;
	HANDLE waitHandle;
};

class DirectoryWinAPI : public Directory
{
public:
	DirectoryWinAPI();
	virtual ~DirectoryWinAPI();

private:
	virtual bool setWatch(const std::string& path, IDirectoryCallbackBase* cb);
	std::map<void*, DirWinAPIWait> callbacks;

	static unsigned int __stdcall threadEntry(void* args);
	static void __stdcall onEventTriggered(void* args, BOOLEAN timeout);
	void update();
	//bool stopping;
	//CRITICAL_SECTION csCallbacks;
};