#include <Common.h>
#include "DirectoryWinAPI.h"

#include "../Common/Logger.h"

#include <process.h>

DirectoryWinAPI::DirectoryWinAPI()
{
	//ZeroMemory(&csCallbacks, sizeof(csCallbacks));
	//InitializeCriticalSection(&csCallbacks);
}

DirectoryWinAPI::~DirectoryWinAPI()
{
	for(auto it = callbacks.begin(); it != callbacks.end(); ++it)
	{
		UnregisterWaitEx(it->second.waitHandle,  INVALID_HANDLE_VALUE);
		FindCloseChangeNotification(it->first);
		delete it->second.cb;
	}
}

bool DirectoryWinAPI::setWatch(const std::string& path, IDirectoryCallbackBase* cb)
{
	if(callbacks.size() > MAXIMUM_WAIT_OBJECTS)
	{
		LOGERROR("Too many objects being watched", "setWatch");
	}

	HANDLE watchHandle = FindFirstChangeNotification(path.c_str(), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE);
	if (watchHandle == INVALID_HANDLE_VALUE) 
	{
		DWORD err = GetLastError();
		LOGERROR(err, "FindFirstChangeNotification");
		return false;
	}

	HANDLE poolHandle;
	if(!RegisterWaitForSingleObject(&poolHandle, watchHandle, onEventTriggered, watchHandle, INFINITE, WT_EXECUTEINWAITTHREAD))
	{
		DWORD err = GetLastError();
		LOGERROR(err, "RegisterWaitForSingleObject");
		return false;
	}

	DirWinAPIWait waitStruct = { cb, poolHandle };
	callbacks[watchHandle] = waitStruct;

	return true;
}

void __stdcall DirectoryWinAPI::onEventTriggered(void* args, BOOLEAN /*timeout*/)
{
	HANDLE handle = (HANDLE)args;
	static_cast<DirectoryWinAPI*>(Directory::get())->callbacks[handle].cb->run();
	FindNextChangeNotification(handle);
	FindNextChangeNotification(handle); //twice
}

/*unsigned int DirectoryWinAPI::threadEntry(void* args)
{
	
	return 0;
}

void DirectoryWinAPI::update()
{
	HANDLE handles[MAXIMUM_WAIT_OBJECTS] = {0};
	IDirectoryCallbackBase* handlesCB[MAXIMUM_WAIT_OBJECTS] = {0};

	while(!stopping)
	{
		DWORD x = 0;
		EnterCriticalSection(&csCallbacks);
		for(auto it = callbacks.begin(); it != callbacks.end(); ++it)
		{
			handles[x] = it->first;
			handlesCB[x] = it->second;
			x++;
		}
		LeaveCriticalSection(&csCallbacks);

		DWORD waitResult = WaitForMultipleObjects(x, handles, FALSE, 2000);
		switch(waitResult)
		{
		case WAIT_FAILED:
			{
				stopping = true;
				DWORD err =GetLastError();
				LOGERROR(err, "WaitForMultipleObjects");
			} break;
		default:
			if(waitResult >= WAIT_OBJECT_0 && waitResult < x)
			{
				handlesCB[waitResult]->run();
				FindNextChangeNotification(handles[waitResult]); 
				FindNextChangeNotification(handles[waitResult]); //twice
			}
		}
	}
}*/