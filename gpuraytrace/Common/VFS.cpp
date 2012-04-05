#include <Common.h>
#include "VFS.h"

#include <sys/stat.h>

VFS::VFS()
{
}

VFS::~VFS()
{
}

void VFS::addPath(const std::string& path)
{
	paths.push_back(path);
}

bool VFS::openFile(const std::string& path, std::string* out)
{
	for(auto it = paths.rbegin(); it != paths.rend(); ++it)
	{
		struct stat st;
		*out = *it + "/" + path;
		if(!::stat(out->c_str(), &st))
		{
			return true;
		}
	}
	return false;
}