#include "Common.h"
#include "WindowWinAPI.h"

WindowWinAPI::WindowWinAPI() : IWindow(WindowAPI::WinAPI)
{
}

WindowWinAPI::~WindowWinAPI()
{

}

bool WindowWinAPI::create()
{
	return false;
}