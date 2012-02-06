#pragma once

#include "Logger.h"

class Factory
{
protected:
	Factory() { }

	template<class T>
	static void create(T** t)
	{
		if(!(*t)->create())
		{
			Logger() << "Could not create " << typeid(t).name();

			delete *t;
			*t = nullptr;
		} else {
			Logger() << "Created " << typeid(t).name();
		}
	}
};