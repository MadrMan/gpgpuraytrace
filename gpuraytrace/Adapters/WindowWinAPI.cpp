#include <Common.h>
#include "WindowWinAPI.h"

#include "../Logger.h"

const int WINDOWLONG_THISPTR = 0;

WindowWinAPI::WindowWinAPI(const WindowSettings& windowSettings) : IWindow(WindowAPI::WinAPI, windowSettings)
{
	closed = false;
	hWnd = 0;
	input = nullptr;
}

WindowWinAPI::~WindowWinAPI()
{
	
}

LRESULT CALLBACK WindowWinAPI::SMainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	WindowWinAPI* window = reinterpret_cast<WindowWinAPI*>(GetWindowLongPtr(hWnd,WINDOWLONG_THISPTR));
	if(window)
		return window->MainWndProc(Msg, wParam, lParam);
	else
		return DefWindowProc(hWnd,Msg,wParam,lParam);
}

LRESULT CALLBACK WindowWinAPI::MainWndProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(input && input->handleWindowMessage(Msg, wParam, lParam))
		return 0;

	switch(Msg)
	{
		case WM_CLOSE: 
			PostQuitMessage(0); 
			closed = true;
			break;	
		case WM_QUIT:
			return 0;
	}

	return DefWindowProc(hWnd,Msg,wParam,lParam);
}

static ATOM WindowWinAPI_classAtom;
bool WindowWinAPI::createWindow()
{
	HMODULE hInstance = GetModuleHandle(0);

	if(!WindowWinAPI_classAtom)
	{
		WNDCLASSEX wcx = {0};
 
		// Fill in the window class structure with parameters 
		// that describe the main window. 
		wcx.cbSize = sizeof(wcx);          // size of structure 
		wcx.style = CS_HREDRAW | CS_VREDRAW;
		wcx.lpfnWndProc = SMainWndProc;
		wcx.hInstance = hInstance;
		wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		//wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcx.lpszClassName = "RaytraceClass";	// name of window class 
		wcx.cbWndExtra = sizeof(LONG) + sizeof(int);	//extra space for a pointer to Window

		// Register the window class. 
		WindowWinAPI_classAtom = RegisterClassEx(&wcx); 

		if(!WindowWinAPI_classAtom)
		{
			DWORD error = GetLastError();
			LOGERROR(error, "RegisterClassEx");
			return false;
		}
	}

    hWnd = CreateWindow( 
        reinterpret_cast<LPCSTR>(WindowWinAPI_classAtom),        // name of window class 
        "Raytracer 1.0",            // title-bar string 
        WS_OVERLAPPEDWINDOW, // top-level window 
        CW_USEDEFAULT,       // default horizontal position 
        CW_USEDEFAULT,       // default vertical position 
		getWindowSettings().width,       // default width 
        getWindowSettings().height,       // default height 
        (HWND)0,         // no owner window 
        (HMENU)0,        // use class menu 
		hInstance,   // handle to application instance 
        (LPVOID)0);      // no window-creation data
	if(!hWnd)
	{
		DWORD error = GetLastError();
		LOGERROR(error, "CreateWindow");
		return false;
	}

	SetLastError(0);
	if(!SetWindowLongPtr(hWnd, WINDOWLONG_THISPTR, reinterpret_cast<LONG>(this)))
	{
		DWORD error = GetLastError();
		if(error)
		{
			LOGERROR(error, "GetWindowLongPtr");

			DestroyWindow(hWnd);
			hWnd = 0;

			return false;
		}
	}

	return true;
}

bool WindowWinAPI::create()
{
	if(!createWindow())
		return false;

	return true;
}

void WindowWinAPI::show()
{
	ShowWindow(hWnd, SW_SHOWDEFAULT); 
    UpdateWindow(hWnd); 
}

bool WindowWinAPI::update()
{
	MSG msg;
	while(PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}

	if(input) input->update();

	return closed;
}

IInput* WindowWinAPI::getInput()
{
	if(!input) input = new InputWinAPI();
	return input;
}