#pragma once

namespace WindowAPI { enum T
{
	WinAPI,
	X11
};}

class IWindow
{
public:
	WindowAPI::T getAPI() const { return api; }

	virtual bool create() = 0;

protected:
	IWindow(WindowAPI::T api) : api(api) { }
	
private:
	WindowAPI::T api;
};

