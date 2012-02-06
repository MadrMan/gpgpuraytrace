#pragma once

#include "IWindow.h"

class WindowWinAPI : public IWindow
{
public:
	WindowWinAPI();
	virtual ~WindowWinAPI();

	virtual bool create() override;
};

