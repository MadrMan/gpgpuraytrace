#pragma once

namespace WindowAPI { enum T
{
	WinAPI,
	X11
};}

struct WindowSettings
{
	int width;
	int height;
};

class IWindow
{
public:
	WindowAPI::T getAPI() const { return api; }
	const WindowSettings& getWindowSettings() const { return windowSettings; }

	virtual bool create() = 0; 
	virtual void show() = 0;

	//! Update the window and handle window messages
	//! \return True when window is running normally, false when the window is closed
	virtual bool update() = 0;
protected:
	IWindow(WindowAPI::T api, const WindowSettings& windowSettings) : api(api), windowSettings(windowSettings) { }
	
private:
	WindowAPI::T api;
	WindowSettings windowSettings;
};