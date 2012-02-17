#include <Common.h>

#include "Raytracer.h"
#include "./Common/Logger.h"
#include "VariableManager.h"

int main(int argc, char** argv)
{
	argc; argv;

	Logger() << "Startup";

	VariableManager::get()->start();

	Variable v;
	
	float test = 4.0;
	v.name = "test";
	v.sizeInBytes = sizeof(float);
	v.type = "float";
	v.pointer = &test;

	VariableManager::get()->registerVariable(v);

	


	for(;;){
		Logger() << test;
		Sleep(100);
	}

	Raytracer* raytracer = new Raytracer();
	raytracer->run();
	delete raytracer;
	return 0;
}