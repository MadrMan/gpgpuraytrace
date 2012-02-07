#include "Common.h"

#include "Raytracer.h"
#include "Logger.h"

int main(int argc, char** argv)
{
	argc; argv;

	Logger() << "Startup";

	Raytracer* raytracer = new Raytracer();
	raytracer->run();
	delete raytracer;

	return 0;
}