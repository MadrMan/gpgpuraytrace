#pragma once

#include "IWindow.h"
#include "Factory.h"

//! Factory used to create new windows
class WindowFactory : private Factory
{
public:
	//! Construct a new window
	//! \param api API to create the new window with
	//! \param windowSettings Settings to create the new window with
	//! \return nullptr if there was an error, otherwise a valid window
	static IWindow* construct(WindowAPI::T api, const WindowSettings& windowSettings);
};