#include <Common.h>
#include "WindowFactory.h"

#include "../Adapters/WindowWinAPI.h"

IWindow* WindowFactory::construct(WindowAPI::T api, const WindowSettings& windowSettings)
{
	IWindow* window = nullptr;

	switch(api)
	{
	case WindowAPI::WinAPI:
		window = new WindowWinAPI(windowSettings);
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