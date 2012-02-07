#pragma once

#include "IWindow.h"

#include <Windows.h>

class WindowWinAPI : public IWindow
{
public:
	WindowWinAPI(const WindowSettings& windowSettings);
	virtual ~WindowWinAPI();

	virtual bool create() override;
	virtual void show() override;
	virtual bool update() override;

	HWND getHandle() const { return hWnd; };

private:
	bool createWindow();
	HWND hWnd;
	bool closed;

	static LRESULT CALLBACK SMainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK MainWndProc(UINT Msg, WPARAM wParam, LPARAM lParam);
};

