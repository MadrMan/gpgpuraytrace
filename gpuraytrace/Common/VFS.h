#pragma once

#include <string>
#include <fstream>

class VFS
{
public:
	static VFS* get()
	{ 
		static VFS instance;
		return &instance;
	}

	void addPath(const std::string& path);
	bool openFile(const std::string& path, std::string* out);

private:
	VFS();
	virtual ~VFS();

	std::vector<std::string> paths;
};