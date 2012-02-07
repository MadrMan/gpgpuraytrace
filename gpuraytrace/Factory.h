#pragma once

#include "Logger.h"

class Factory
{
protected:
	Factory() { }

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