#include "Common.h"
#include "Raytracer.h"

int main(int argc, char** argv)
{
	Raytracer* raytracer = new Raytracer();
	raytracer->run();
	delete raytracer;

	return 0;
}