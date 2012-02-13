#include <Common.h>
#include "InputWinAPI.h"

#include "../Logger.h"

InputActionWinAPI::InputActionWinAPI(InputWinAPI* input) : input(input)
{ 
}

float InputActionWinAPI::getState()
{
	return state;
}

void InputActionWinAPI::update()
{
	state = 0.0f;

	for(auto it = keyboardKeys.begin(); it != keyboardKeys.end(); ++it)
	{
		if(input->keys[it->key])
		{
			state += it->value;
		}
	}
	for(auto it = mouseButtons.begin(); it != mouseButtons.end(); ++it)
	{
		if(input->mouseButtons[it->key])
		{
			state += it->value;
		}
	}
	for(auto it = mouseAxis.begin(); it != mouseAxis.end(); ++it)
	{
		state += input->mouseAxis[*it];
	}
}

void InputActionWinAPI::registerKeyboard(int key, float highValue)
{
	KeyboardTrigger kt = {key, highValue};
	keyboardKeys.push_back(kt);
}

void InputActionWinAPI::registerMouseButton(MouseButtons::T button, float highValue)
{
	MouseTrigger mt = {(int)button, highValue};
	mouseButtons.push_back(mt);
}

void InputActionWinAPI::registerMouseAxis(int axis)
{
	mouseAxis.push_back(axis);
}



InputWinAPI::InputWinAPI()
{
	ZeroMemory(keys, sizeof(keys));
	ZeroMemory(mouseButtons, sizeof(mouseButtons));
	ZeroMemory(mouseAxis, sizeof(mouseAxis));
	mouseXPos = 0;
	mouseYPos = 0;
}

InputWinAPI::~InputWinAPI()
{

}

void InputWinAPI::update()
{
	IInput::update();
}


bool InputWinAPI::handleWindowMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
		case WM_KEYDOWN:
		{
			keys[wParam] = 1;
			return true;
		}
		case WM_KEYUP:
		{
			keys[wParam] = 0;
			return true;
		}
		case WM_MOUSEMOVE:
		{
			int xpos = LOWORD(lParam);
			int ypos = HIWORD(lParam);
			int xDelta = xpos - mouseXPos;
			int yDelta = ypos - mouseYPos;
			mouseXPos = xpos;
			mouseYPos = ypos;
			mouseAxis[0] = (float)xDelta;
			mouseAxis[1] = (float)yDelta;
			return true;
		}
		case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			mouseAxis[2] = (float)zDelta;
			return true;
		}
		case WM_LBUTTONDOWN:
			mouseButtons[MouseButtons::LeftButton] = 1;
			return true;
		case WM_RBUTTONDOWN:
			mouseButtons[MouseButtons::MiddleButton] = 1;
			return true;
		case WM_MBUTTONDOWN:
			mouseButtons[MouseButtons::MiddleButton] = 1;
			return true;
		case WM_RBUTTONUP:
			mouseButtons[MouseButtons::RightButton] = 0;
			return true;
		case WM_MBUTTONUP:
			mouseButtons[MouseButtons::MiddleButton] = 0;
			return true;
		case WM_LBUTTONUP:
			mouseButtons[MouseButtons::LeftButton] = 0;
			return true;
	}
	return false;
}



IInputAction* InputWinAPI::createAction()
{
	InputActionWinAPI* action = new InputActionWinAPI(this);
	actions.push_back(action);
	return action;
}