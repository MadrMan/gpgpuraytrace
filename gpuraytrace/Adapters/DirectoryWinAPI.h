#pragma once

#include "../Common/Directory.h"

#include <map>

//! Struct used by DirectoryWinAPI to contain the callbacks
struct DirWinAPIWait
{
	ICallbackBase<void>* cb;
	HANDLE waitHandle;
};

//! Directory handler for windows
//! Uses the windows thread pool to receive events from the filesystem
class DirectoryWinAPI : public Directory
{
public:
	DirectoryWinAPI();
	virtual ~DirectoryWinAPI();

private:
	virtual bool setWatch(const std::string& path, ICallbackBase<void>* cb);
	std::map<void*, DirWinAPIWait> callbacks;

	static unsigned int __stdcall threadEntry(void* args);
	static void __stdcall onEventTriggered(void* args, BOOLEAN timeout);
	void update();
	//bool stopping;
	//CRITICAL_SECTION csCallbacks;
};