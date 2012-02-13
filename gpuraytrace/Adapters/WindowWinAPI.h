#pragma once

#include "../Factories/IWindow.h"
#include "InputWinAPI.h"

#include <Windows.h>

//! A window to be used on Windows
class WindowWinAPI : public IWindow
{
public:
	//! Constructor
	//! \param windowSettings Settings to create the new window with
	WindowWinAPI(const WindowSettings& windowSettings);

	//! Destructor
	virtual ~WindowWinAPI();

	//! Create the window using the parameters supplied to the constructor
	//! \return True if successful, false if not
	virtual bool create() override;

	//! Make the window visible
	virtual void show() override;

	//! Update the window and handle window messages
	//! \return True when window is running normally, false when the window is closed
	virtual bool update() override;

	//! Get the window handle to the created window
	//! \return A valid handle if the window was constructed, otherwise 0
	HWND getHandle() const { return hWnd; };

	//! Get the input handler for the window
	virtual IInput* getInput() override;

private:
	bool createWindow();
	HWND hWnd;
	bool closed;

	InputWinAPI* input;

	static LRESULT CALLBACK SMainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK MainWndProc(UINT Msg, WPARAM wParam, LPARAM lParam);
};

