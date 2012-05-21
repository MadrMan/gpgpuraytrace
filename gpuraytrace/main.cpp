#include <Common.h>

#include "Raytracer.h"
#include "./Common/Logger.h"
#include "./Common/VariableManager.h"
#include "./Common/Settings.h"

void displayHelp()
{
		Logger() << "Usage: gpgpuraytrace [-r=800x600][-c][-f][-d][-t][-l=landscape]";
		Logger() << "-r: set resolution.";
		Logger() << "-c: capture mode: enables fly-by mode and recording";
		Logger() << "-f: fullscreen mode.";
		Logger() << "-d: enable day/night cyclus.";
		Logger() << "-t: enable remote config tool.";

		std::string commandL("-l: set landscape e.g. ");
		auto it = landscapes.begin();
		for(; it != landscapes.end(); ++it)
		{
			commandL += it->name + " ";
		}
		Logger() << commandL;  
}


bool processParameter(Mode* mode, Landscape* landscape, const std::string& str)
{
	const std::string PARAM_PREFIX = "parameter: ";
	const std::string PARAM_PREFIX_ERROR = "parameter error: ";
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
				convert(value.substr(split + 1), &mode->ws.height) &&
				mode->ws.width > 0 && mode->ws.height > 0)
			{
				Logger() << PARAM_PREFIX << "resolution = " << mode->ws.width << "x" << mode->ws.height;
			} else {
				Logger() << PARAM_PREFIX_ERROR  << "invalid resolution: " << value;
				return false;
			}
		} else {
			Logger() << PARAM_PREFIX_ERROR << "Invalid value supplied to -r.";
			return false;
		}
	} else if(command == "-c") {
		mode->recordMode = true;
		mode->fixedFrameRate = true;
		mode->step_mod = STEP_MOD_RECORD;
		Logger() << PARAM_PREFIX << "record mode enabled.";
	} else if(command == "-f") {
		mode->ws.fullscreen = true;
		Logger() << PARAM_PREFIX << "fullscreen enabled.";
	} else if(command == "-t") {
		mode->enableManager = true;
		Logger() << PARAM_PREFIX << "manager enabled.";
	} else if(command == "-l") {
		auto it = landscapes.begin();
		for(; it != landscapes.end(); ++it)
		{
			if(it->name == value)
			{
				Logger() << PARAM_PREFIX << "Landscape " << value << " selected.";
				*landscape = *it;
				break;
			}
		}

		if(it == landscapes.end())
		{
			Logger() << PARAM_PREFIX_ERROR << "Landscape " << value << " was not found.";
			return false;
		}
	} else if(command == "-d") {
		mode->incrementDayTime = true;
		Logger() << PARAM_PREFIX << "day/night cyclus enabled.";
	} else {
		Logger() << PARAM_PREFIX_ERROR << "Unknown command \n" ;
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
#ifdef _DEBUG
	Mode mode =  MODE_BENCHMARK; //Take standard settings from this mode
#else
	Mode mode = MODE_RELEASE;
#endif
	Landscape landscape = landscapes[0];

	for(int x = 1; x < argc; x++)
	{
		if(!processParameter(&mode, &landscape, argv[x]))
		{
			displayHelp();
			return 0;
		}
	}
	Logger() << "=== Startup";

	if(mode.enableManager) VariableManager::get()->start();

	Raytracer* raytracer = new Raytracer();
	raytracer->run(mode, landscape);

	delete raytracer;
	return 0;
}