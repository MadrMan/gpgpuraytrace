#include "Common.h"
#include "WindowFactory.h"

#include "WindowWinAPI.h"

IWindow* WindowFactory::construct(WindowAPI::T api)
{
	IWindow* window = nullptr;

	switch(api)
	{
	case WindowAPI::WinAPI:
		window = new WindowWinAPI();
		break;
	case WindowAPI::X11:
		break;
	default:
		break;
	}

	if(window)
	{
		create(&window);
	}

	return window;
}