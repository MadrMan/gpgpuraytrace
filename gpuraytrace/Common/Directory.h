#pragma once

#include <string>

//! Directory handler which is used for managing directories
class Directory
{
public:
	//! Get the Directory object
	static Directory* get();

	//! Watch a directory for changes
	//! \param obj Pointer to the object in which the callback function is
	//! \param fptr Pointer to the function which will receive the callback
	template<class T>
	bool watch(const std::string& path, T* obj, typename ICallback<T, void>::Fptr fptr)
	{
		return setWatch(path, new ICallback<T, void>(obj, fptr));
	}

	//! Return all the files in a directory
	//bool getFiles(std::vector<std::string>* filesOut, bool recursive);

private:
	virtual bool setWatch(const std::string& path, ICallbackBase<void>* cb) = 0;
	static Directory* directory;
};