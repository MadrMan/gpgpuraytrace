#pragma once

#include <string>

class IDirectoryCallbackBase
{
public:
	virtual void run() = 0;
};

template<class T>
class IDirectoryCallback : public IDirectoryCallbackBase
{
public:
	typedef void (T::*Fptr)();

	IDirectoryCallback(T* obj, Fptr fptr) : obj(obj), fptr(fptr) 
	{ }

	virtual void run() override
	{ (obj->*fptr)(); }

private:
	T* obj;
	Fptr fptr;
};

class Directory
{
public:
	static Directory* get();

	//! Watch a directory for changes
	//! \param 
	template<class T>
	bool watch(const std::string& path, T* obj, typename IDirectoryCallback<T>::Fptr fptr)
	{
		return setWatch(path, new IDirectoryCallback<T>(obj, fptr));
	}

	//! Return all the files in a directory
	//bool getFiles(std::vector<std::string>* filesOut, bool recursive);

private:
	virtual bool setWatch(const std::string& path, IDirectoryCallbackBase* cb) = 0;
	static Directory* directory;
};