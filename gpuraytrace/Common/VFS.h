#pragma once

#include <string>
#include <fstream>

//! Virtual File System class
class VFS
{
public:
	//! Get a handle to the VFS
	static VFS* get()
	{ 
		static VFS instance;
		return &instance;
	}

	//! Add a root path to the VFS
	void addPath(const std::string& path);

	//! Open a file using the VFS searching trough all the root paths
	bool openFile(const std::string& path, std::string* out);

private:
	VFS();
	virtual ~VFS();

	std::vector<std::string> paths;
};