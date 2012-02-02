#include <Windows.h>

struct color
{
	BYTE r;
	BYTE g;
	BYTE b;
};

const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 500;

color imagedata[WINDOW_HEIGHT * WINDOW_WIDTH * 3];
HBITMAP memBitmap;
HDC memHDC;

float xrotation;
float yrotation;
int xposition;
int yposition;

void fillImageData()
{
	color* i = imagedata;
	for(int y = 0; y< WINDOW_HEIGHT; y++)
	{
		for(int x =0; x<WINDOW_WIDTH; x++)
		{
			i->r = -x + xrotation;
			i->g = y + yrotation;
			i->b = -y;
			
			i++;
		}
	}
}

void fillBitmap(HDC hdc, HBITMAP bitmap)
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

     SetDIBits(hdc, bitmap, 0, WINDOW_HEIGHT, &imagedata, &bmi, 0);
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
				memHDC = CreateCompatibleDC(hdc);
				memBitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
			 }
			return 0;
		case WM_CLOSE: 
			PostQuitMessage(0); 
			break;	
		case WM_ERASEBKGND:
			return 0;
		case WM_PAINT:
			{
				HDC hdc = BeginPaint(hWnd, &ps);
				
				HBITMAP oldMemBmp = (HBITMAP)SelectObject(memHDC, memBitmap);
				BitBlt(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, memHDC, 0, 0, SRCCOPY);
				SelectObject(memHDC, oldMemBmp);

				//DeleteDC(memhdc);
				EndPaint(hWnd, &ps);

				//HDC memhdc = CreateCompatibleDC(GetDC(hWnd));

				///SelectObject(memhdc, hbitmap);
				//fillBitmap(memhdc);
				//SelectObject(memhdc, oldMemBmp);
			}
			return 0;
		 case WM_MOUSEMOVE:
			{
				int xpos = LOWORD(lParam);
				int ypos = HIWORD(lParam);
				int xmove = xpos - xposition;
				int ymove = ypos - yposition;
				xposition = xpos;
				yposition = ypos;

				xrotation += ((float)xmove) * 0.1f;
				yrotation += ((float)ymove) * 0.1f;

				fillImageData();
				fillBitmap(memHDC, memBitmap);

				InvalidateRect(hWnd, 0, TRUE);
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
	
	MSG msg;
    while (GetMessage(&msg, (HWND) NULL, 0, 0)) 
    { 
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    } 
}