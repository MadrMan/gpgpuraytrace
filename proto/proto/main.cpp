#define _USE_MATH_DEFINES

#include <Windows.h>
#include <iostream>
#include <d3dx9.h>

#include <math.h>

struct Color
{
	Color() 
	{
		r = g = b = 0;
	}
	Color(BYTE r, BYTE g, BYTE b) : r(r), g(g), b(b)
	{
	}
	BYTE r;
	BYTE g;
	BYTE b;
};

const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 500;
bool leftButtonDown = false;
bool rightButtonDown = false;
bool isSpaceDown = true;
float moveStep = 0.1f;
bool quit = false;

float timeConstant;
DWORD tickCount;
DWORD refresher;

Color imagedata[WINDOW_HEIGHT * WINDOW_WIDTH * 3];
HBITMAP memBitmap;
HDC memHDC;

D3DXMATRIX camProjection;
D3DXMATRIX camView;
D3DXMATRIX camInvView;
D3DXMATRIX camVP;

const int cubeTris = 12;

const int cubeIndices[] = {
	0, 1, 2,
	1, 2, 3,
	0, 1, 4,
	1,4,5,
	5,3,7,
	4,5,7,
	7,6,4,
	0,4,2,
	2,4,6,
	2,6,3,
	3,6,7

};

const D3DXVECTOR3 cube[] = {
	D3DXVECTOR3(-0.5f, 0.5f, 0.5f),		//0
	D3DXVECTOR3(0.5f, 0.5f, 0.5f),		//1
	D3DXVECTOR3(-0.5f ,-0.5f , 0.5f),	//2
	D3DXVECTOR3(0.5f, -0.5f, 0.5f),		//3

	D3DXVECTOR3(-0.5f, 0.5f, -0.5f),	//4
	D3DXVECTOR3(0.5f, 0.5f, -0.5f),		//5
	D3DXVECTOR3(-0.5f, -0.5f, -0.5f),	//6
	D3DXVECTOR3(0.5f, -0.5f, -0.5f),	//7
};

float xrotation;
float yrotation;
int xposition;
int yposition;
D3DXVECTOR3 camPosition(10.0f, 10.0f, 10.0f);
D3DXVECTOR3 camDirection;

void getRay(D3DXVECTOR3* rayStart, D3DXVECTOR3* rayDirection, const D3DXVECTOR2& screenPos)
{
	D3DXVECTOR3 screenPosScaled((screenPos.x / WINDOW_WIDTH - 0.5f) * 2.0f, (screenPos.y / WINDOW_HEIGHT - 0.5f) * 2.0f, 1.0f);

	screenPosScaled.x /= camProjection._11;
	screenPosScaled.y /= camProjection._22;

	D3DXVECTOR4 rayPoint4;
	D3DXVec3Transform(&rayPoint4, &screenPosScaled, &camInvView);
	*rayStart = D3DXVECTOR3(rayPoint4.x, rayPoint4.y, rayPoint4.z);
	D3DXVECTOR3 rayVec = *rayStart - camPosition;
	D3DXVec3Normalize(rayDirection, &rayVec);
}

void camera()
{
	const D3DXVECTOR3 camForward(0.0f, 0.0f, 1.0f);
	const D3DXVECTOR3 camUp(0.0f, 1.0f, 0.0f);

	D3DXMATRIX rotationMatrix;
	D3DXVECTOR4 camDirection4;
	
	D3DXMatrixRotationYawPitchRoll(&rotationMatrix, xrotation, yrotation, 0.0f);
	D3DXVec3Transform(&camDirection4, &camForward, &rotationMatrix);
	camDirection.x = camDirection4.x;
	camDirection.y = camDirection4.y;
	camDirection.z = camDirection4.z;

	if(leftButtonDown) camPosition += camDirection * moveStep * timeConstant;
	if(rightButtonDown) camPosition -= camDirection * moveStep * timeConstant;

	D3DXVec3Normalize(&camDirection, &camDirection);
	D3DXVECTOR3 camLookat  = camPosition + camDirection;

	D3DXMatrixLookAtLH(&camView, &camPosition, &camLookat, &camUp);
	D3DXMatrixMultiply(&camVP, &camView, &camProjection);
	D3DXMatrixInverse(&camInvView, 0, &camView);
}

void setPixel(int x, int y, const Color& rgb)
{
	if(x < 0 || y < 0 || x >= WINDOW_WIDTH || y >= WINDOW_HEIGHT)
		return;
	imagedata[y * WINDOW_HEIGHT + x].r |= rgb.r;
	imagedata[y * WINDOW_HEIGHT + x].g |= rgb.g;
	imagedata[y * WINDOW_HEIGHT + x].b |= rgb.b;
}

void drawLine(int x1,int y1, int x2, int y2, const Color& rgb)
{
	float xdist = (float)(x2 - x1);
	float ydist = (float)(y2 - y1);
	float dist = sqrtf(xdist * xdist + ydist * ydist);
	float xstep = xdist / dist;
	float ystep = ydist / dist;

	for(int d = 0; d < dist; d++)
	{
		setPixel((int)(x1 + xstep * d), (int)(y1 + ystep * d), rgb);
	}
}

void getRayCollision(const D3DXVECTOR3& rayPoint, float* density)
{
	if(fabs(rayPoint.x) < 0.5f &&
		fabs(rayPoint.y) < 0.5f &&
		fabs(rayPoint.z) < 0.5f)
	{
		*density = 4.0f;
	} else {
		*density = 0.0f;
	}
}

void fillImageData()
{
	Color* i = imagedata;
	ZeroMemory(i, WINDOW_HEIGHT * WINDOW_HEIGHT * sizeof(Color));

	if(isSpaceDown)
	{
		for(int x = 0; x < cubeTris; x++)
		{
			const D3DXVECTOR3& v1 = cube[cubeIndices[x * 3 + 0]];
			const D3DXVECTOR3& v2 = cube[cubeIndices[x * 3 + 1]];
			const D3DXVECTOR3& v3 = cube[cubeIndices[x * 3 + 2]];

			D3DXVECTOR4 v1t, v2t, v3t;
			D3DXVec3Transform(&v1t, &v1, &camVP);
			D3DXVec3Transform(&v2t, &v2, &camVP);
			D3DXVec3Transform(&v3t, &v3, &camVP);


			v1t /= v1t.w;
			v2t /= v2t.w;
			v3t /= v3t.w;

			v1t *= 0.5f;
			v1t.x += 0.5f;
			v1t.y += 0.5f;
			v1t.x *= WINDOW_WIDTH; 
			v1t.y *= WINDOW_HEIGHT;
		
			v2t *= 0.5f;
			v2t.x += 0.5f;
			v2t.y += 0.5f;
			v2t.x *= WINDOW_WIDTH; 
			v2t.y *= WINDOW_HEIGHT;

			v3t *= 0.5f;
			v3t.x += 0.5f;
			v3t.y += 0.5f;
			v3t.x *= WINDOW_WIDTH; 
			v3t.y *= WINDOW_HEIGHT;

			//-1.-1   1,1
			//
			drawLine((int)v1t.x, (int)v1t.y, (int)v2t.x, (int)v2t.y, Color(255, 0, 0));
			drawLine((int)v2t.x, (int)v2t.y, (int)v3t.x, (int)v3t.y, Color(0, 255, 0));
			drawLine((int)v3t.x, (int)v3t.y, (int)v1t.x, (int)v1t.y, Color(0, 0, 255));
			//setPixel((int)v1t.x, (int)v1t.y, Color(255, 0, 0));
			//setPixel((int)v2t.x, (int)v2t.y, Color(0, 255, 0));
			//setPixel((int)v3t.x, (int)v3t.y, Color(0, 0, 255));
		}
	} else {
		D3DXVECTOR3 rayPoint;
		D3DXVECTOR3 rayDirection;

		const float RAY_DISTANCE = 10.0f;
		const float RAY_STEP = 0.01f;
		const int RAY_STEPS = (int)(RAY_DISTANCE / RAY_STEP);

		for(int y = 0; y < WINDOW_HEIGHT; y++)
		{
			for(int x = 0; x < WINDOW_WIDTH; x++)
			{
				/*i->r = -x + xrotation;
				i->g = y + yrotation;
				i->b = -y;*/

				float density = 0.0f;
				if(x > 100 && x < 400 && y > 100 && y < 400)
				{
					float rayDensity = 0.0f;

					getRay(&rayPoint, &rayDirection, D3DXVECTOR2((float)x, (float)y));
					for(int r = 0; r < RAY_STEPS; r++)
					{
						getRayCollision(rayPoint, &rayDensity);
						density += rayDensity * RAY_STEP;

						/*if(density > 1.0f) 
						{
							density = 1.0f;
							break;
						}*/

						rayPoint += rayDirection * RAY_STEP;
					}
				}

				i->r = min((int)(density * 10.0f), 255);
				i->g = min((int)(density * 100.0f), 255);
				i->b = min((int)(density * 300.0f), 255);

				i++;
			}
		}
	}
}

void render()
{
	camera();
	fillImageData();
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

				D3DXMatrixPerspectiveFovLH(&camProjection, (float)M_PI * 0.25f, WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
			 }
			return 0;
		case WM_CLOSE: 
			PostQuitMessage(0); 
			quit = true;
			break;	
		case WM_QUIT:
			return 0;
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

				if(!(wParam & MK_CONTROL))
				{
					xrotation += ((float)xmove) * 0.02f;
					yrotation += ((float)ymove) * 0.02f;
				}
			}
			return 0;
		 case WM_KEYUP:
		 case WM_KEYDOWN:
			{
				bool state = false;
				if(Msg == WM_KEYDOWN)
					state = true;

				switch(wParam)
				{
				case VK_SPACE:
					isSpaceDown = state;
					break;
				}
			}
			return 0;
		 case WM_LBUTTONDOWN:
			 {
				 leftButtonDown = true;
			 }
			 return 0;
		 case WM_LBUTTONUP:
			 {
				 leftButtonDown = false;
			 }
			 return 0;
		 case WM_RBUTTONUP:
			 {
				 rightButtonDown = false;
			 }
			 return 0;
		 case WM_RBUTTONDOWN:
			 {
				 rightButtonDown = true;
			 }
			 return 0;

	

	}
	
	
	return DefWindowProc(hWnd,Msg,wParam,lParam);
}

HWND createWindow()
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

	return hwnd;
}

const int fps = 1000 / 60;
int main(char** argv, int argc)
{
	HWND hWnd = createWindow();
	
	MSG msg;
	while(!quit)
	{
		DWORD newTickCount = GetTickCount();
		DWORD tickDifference = newTickCount - tickCount;
		tickCount = newTickCount;
		refresher += tickDifference;
		if(refresher >= fps)
		{
			InvalidateRect(hWnd, 0, TRUE);
			UpdateWindow(hWnd);

			refresher %= fps;
		}

		timeConstant = (float)tickDifference * 0.1f;
		std::cout << timeConstant << std::endl;

		render();
		fillBitmap(memHDC, memBitmap);

		while (PeekMessage(&msg, (HWND) NULL, 0, 0, PM_REMOVE)) 
		{ 
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
	}
}