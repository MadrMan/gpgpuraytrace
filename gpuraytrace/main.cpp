#include <Common.h>

#include "Raytracer.h"
#include "./Common/Logger.h"
#include "./Common/VariableManager.h"
#include "./Common/Settings.h"

bool processParameter(Mode* mode, const std::string& str)
{
	size_t p = str.find_first_of('=');
	std::string command, value;
	if(p != str.npos)
	{
		command = str.substr(0, p);
		value = str.substr(p + 1);
	} else {
		command = str;
	}

	if(command == "-r")
	{
		size_t split = value.find_first_of('x');
		if(split != str.npos)
		{
			if(convert(value.substr(0, split), &mode->ws.width) &&
				convert(value.substr(split + 1), &mode->ws.height))
			{
				Logger() << "Set resolution to: " << mode->ws.width << "x" << mode->ws.height;
			} else {
				Logger() << "Invalid resolution: " << value;
			}
		} else {
			Logger() << "Invalid value supplied to -r";
		}
	} else if(command == "-c") {
		mode->recordMode = true;
		Logger() << "Enabled record mode";
	} else if(command == "-f") {
		mode->ws.fullscreen = true;
		Logger() << "Enabled fullscreen mode";
	} else {
		Logger() << "Usage: gpgpuraytrace [-r=123x456][-c][-f]";
		Logger() << "-r: set resolution";
		Logger() << "-c: capture to file";
		Logger() << "-f: fullscreen mode";
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
	Mode mode = MODE_TEST; //Take standard settings from this mode
	for(int x = 1; x < argc; x++)
	{
		if(!processParameter(&mode, argv[x]))
		{
			return 0;
		}
	}

	Logger() << "Startup";

	VariableManager::get()->start();

	Raytracer* raytracer = new Raytracer();
	raytracer->run(mode);

	delete raytracer;
	return 0;
}