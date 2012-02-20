#include <Common.h>

#include "Raytracer.h"
#include "./Common/Logger.h"
#include "VariableManager.h"

int main(int argc, char** argv)
{
	argc; argv;

	Logger() << "Startup";

	VariableManager::get()->start();

	Raytracer* raytracer = new Raytracer();
	raytracer->run();

	for(;;){
		Sleep(100);
	}
	delete raytracer;
	return 0;
}