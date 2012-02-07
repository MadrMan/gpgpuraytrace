#pragma once

#include "Logger.h"

//! Class inherited by all the factories, contains helper functions
class Factory
{
protected:
	Factory() { }

	//! Helper function to initialize an interface
	//! \param t Pointer to the interface variable, will be deleted and set to nullptr if there was an error
	template<class T>
	static void create(T** t)
	{
		T& rt = **t;
		if(!(*t)->create())
		{
			Logger() << "Could not create " << typeid(rt).name();

			delete *t;
			*t = nullptr;
		} else {
			Logger() << "Created " << typeid(rt).name();
		}
	}
};