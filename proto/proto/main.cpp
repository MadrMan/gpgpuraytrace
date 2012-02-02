#include <Windows.h>

struct color
{
	BYTE r;
	BYTE g;
	BYTE b;
};

const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 500;
HBITMAP hbitmap;
color imagedata[WINDOW_HEIGHT * WINDOW_WIDTH * 3];

void fillImageData()
{
	color* i = imagedata;
	for(int y = 0; y< WINDOW_HEIGHT; y++)
	{
		for(int x =0; x<WINDOW_WIDTH; x++)
		{
			
			i->r = -x;
			i->g = y;
			i->b = -y;
			
			i++;
		}
	}

}



void fillBitmap(HDC memhdc)
{
	 BITMAPINFO bmi;
     ZeroMemory(&bmi, sizeof(bmi));

     bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
     bmi.bmiHeader.biWidth = WINDOW_WIDTH;
     bmi.bmiHeader.biHeight = WINDOW_HEIGHT;
     bmi.bmiHeader.biPlanes = 1;
     bmi.bmiHeader.biBitCount = 24;
     bmi.bmiHeader.biCompression = BI_RGB;
     bmi.bmiHeader.biSizeImage = 0;
     bmi.bmiHeader.biXPelsPerMeter = 0;
     bmi.bmiHeader.biYPelsPerMeter = 0;
     bmi.bmiHeader.biClrUsed = 0;
     bmi.bmiHeader.biClrImportant = 0;

     SetDIBits(memhdc, hbitmap, 0, WINDOW_HEIGHT, &imagedata, &bmi, 0);
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;


	switch(Msg)
	{
		case WM_CREATE:
			 {
					 // create the bitmap when the window is created
					 HDC hdc = GetDC(hWnd);
					 hbitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
			 }
			return 0;
		case WM_CLOSE: 
			PostQuitMessage(0); 
			break;	
		case WM_PAINT:
			{
				HDC hdc = BeginPaint(hWnd, &ps);
				HDC memhdc = CreateCompatibleDC(hdc);

				SelectObject(memhdc, hbitmap);
				BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, memhdc, 0, 0, SRCCOPY);
				SelectObject(memhdc, 0);

				//DeleteDC(memhdc);
				EndPaint(hWnd, &ps);

				//HDC memhdc = CreateCompatibleDC(GetDC(hWnd));

				SelectObject(memhdc, hbitmap);
				fillBitmap(memhdc);
				SelectObject(memhdc, 0);

				DeleteDC(memhdc);
			}
			return 0;
		 case WM_MOUSEMOVE:
			{


				InvalidateRect(hWnd,0,TRUE);
				UpdateWindow(hWnd);
			}
			return 0;


	}
	
	
	return DefWindowProc(hWnd,Msg,wParam,lParam);
}

void createWindow()
{
	WNDCLASSEX wcx; 
 
    // Fill in the window class structure with parameters 
    // that describe the main window. 
    wcx.cbSize = sizeof(wcx);          // size of structure 
    wcx.style = CS_HREDRAW | 
        CS_VREDRAW;                    // redraw if size changes 
    wcx.lpfnWndProc = MainWndProc;     // points to window procedure 
    wcx.cbClsExtra = 0;                // no extra class memory 
    wcx.cbWndExtra = 0;                // no extra window memory 
    wcx.hInstance = GetModuleHandle(0);         // handle to instance 
    wcx.hIcon = LoadIcon(NULL, 
        IDI_APPLICATION);              // predefined app. icon 
    wcx.hCursor = LoadCursor(NULL, 
        IDC_ARROW);                    // predefined arrow 
    wcx.hbrBackground = (HBRUSH) GetStockObject( 
        WHITE_BRUSH);                  // white background brush 
    wcx.lpszMenuName =  0;
    wcx.lpszClassName = "RaytraceClass";  // name of window class 
    wcx.hIconSm = 0;

    // Register the window class. 
 
    RegisterClassEx(&wcx); 


 
     HWND hwnd = CreateWindow( 
        "RaytraceClass",        // name of window class 
        "Ray trace demo",            // title-bar string 
        WS_OVERLAPPEDWINDOW, // top-level window 
        CW_USEDEFAULT,       // default horizontal position 
        CW_USEDEFAULT,       // default vertical position 
        WINDOW_WIDTH,       // default width 
        WINDOW_HEIGHT,       // default height 
        (HWND) NULL,         // no owner window 
        (HMENU) NULL,        // use class menu 
		wcx.hInstance,           // handle to application instance 
        (LPVOID) NULL);      // no window-creation data 
 
 
 
    // Show the window and send a WM_PAINT message to the window 
    // procedure. 
 
    ShowWindow(hwnd, SW_SHOWDEFAULT); 
    UpdateWindow(hwnd); 

}

int main(char** argv, int argc)
{
	createWindow();
	fillImageData();

	MSG msg;
    while (GetMessage(&msg, (HWND) NULL, 0, 0)) 
    { 
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    } 



}