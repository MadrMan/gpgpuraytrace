#include "Common.h"
#include "WindowWinAPI.h"
#include "Logger.h"

const int WINDOWLONG_THISPTR = 0;

WindowWinAPI::WindowWinAPI(const WindowSettings& windowSettings) : IWindow(WindowAPI::WinAPI, windowSettings)
{
	closed = false;
	hWnd = 0;
}

WindowWinAPI::~WindowWinAPI()
{
	
}

LRESULT CALLBACK WindowWinAPI::SMainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	WindowWinAPI* window = (WindowWinAPI*) GetWindowLongPtr(hWnd,WINDOWLONG_THISPTR);
	if(window)
		return window->MainWndProc(Msg, wParam, lParam);
	else
		return DefWindowProc(hWnd,Msg,wParam,lParam);
}

LRESULT CALLBACK WindowWinAPI::MainWndProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
		case WM_CLOSE: 
			PostQuitMessage(0); 
			closed = true;
			break;	
		case WM_QUIT:
			return 0;
		case WM_CREATE:
			 {
			 }
			return 0;
	}

	return DefWindowProc(hWnd,Msg,wParam,lParam);
}


bool WindowWinAPI::createWindow()
{
	WNDCLASSEX wcx = {0};
 
    // Fill in the window class structure with parameters 
    // that describe the main window. 
    wcx.cbSize = sizeof(wcx);          // size of structure 
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = SMainWndProc;
    wcx.hInstance = GetModuleHandle(0);
    wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    //wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.lpszClassName = "RaytraceClass";	// name of window class 
	wcx.cbWndExtra = sizeof(LONG) + sizeof(int);	//extra space for a pointer to Window

    // Register the window class. 
    RegisterClassEx(&wcx); 

    hWnd = CreateWindow( 
        wcx.lpszClassName,        // name of window class 
        "Raytracer 1.0",            // title-bar string 
        WS_OVERLAPPEDWINDOW, // top-level window 
        CW_USEDEFAULT,       // default horizontal position 
        CW_USEDEFAULT,       // default vertical position 
		getWindowSettings().width,       // default width 
        getWindowSettings().height,       // default height 
        (HWND)0,         // no owner window 
        (HMENU)0,        // use class menu 
		wcx.hInstance,   // handle to application instance 
        (LPVOID)0);      // no window-creation data 

	SetLastError(0);
	if(!SetWindowLongPtr(hWnd, WINDOWLONG_THISPTR, (LONG)this))
	{
		DWORD error = GetLastError();
		if(error)
		{
			LOGERROR(error, "GetWindowLongPtr");
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

	return closed;
}