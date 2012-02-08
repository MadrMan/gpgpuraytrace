#pragma once

namespace WindowAPI { enum T
{
	WinAPI, //!< A Windows WinAPI window type
	X11 //!< An X11 window type usable on Linux/Mac
};}

//! Window settings for creating windows with
struct WindowSettings
{
	int width;
	int height;
};

class IInput;

//! Interface inherited by all the windows
class IWindow
{
public:
	//! Get the API used to create this window with
	WindowAPI::T getAPI() const { return api; }

	//! Get the settings used to create this window with
	const WindowSettings& getWindowSettings() const { return windowSettings; }

	//! Create the window using the parameters supplied to the constructor
	//! \return True if successful, false if not
	virtual bool create() = 0; 

	//! Make the window visible
	virtual void show() = 0;

	//! Update the window and handle window messages
	//! \return True when window is running normally, false when the window is closed
	virtual bool update() = 0;

	//! Get the input handler for the window
	virtual IInput* getInput() = 0;

protected:
	//! Constructor
	IWindow(WindowAPI::T api, const WindowSettings& windowSettings) : api(api), windowSettings(windowSettings) { }
	
private:
	WindowAPI::T api;
	WindowSettings windowSettings;
};