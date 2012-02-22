#include <Common.h>
#include "InputWinAPI.h"

#include "../Common/Logger.h"

InputActionWinAPI::InputActionWinAPI(InputWinAPI* input) : input(input)
{
	state = 0.0f;
	triggered = 0;
	triggeredState = 0;
}

float InputActionWinAPI::getState() const
{
	return state;
}

bool InputActionWinAPI::isTriggered() const
{
	return triggered;
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

	/*if(isnull(state))
	{
		if(!triggered)
		{
			triggered = false;
			triggeredState = TriggerType::OnRelease;
		} else {
			triggeredState = TriggerType::;
		}
	} else {
		if(!triggered)
		{
			triggered = true;
		}
	}*/
}

void InputActionWinAPI::registerKeyboard(int key, float highValue, TriggerType::T trigger)
{
	KeyboardTrigger kt = {key, highValue, trigger};
	keyboardKeys.push_back(kt);
}

void InputActionWinAPI::registerMouseButton(MouseButtons::T button, float highValue, TriggerType::T trigger)
{
	MouseTrigger mt = {(int)button, highValue, trigger};
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

	for(int x = 0; x < _countof(mouseAxis); x++)
	{
		mouseAxis[x] = 0.0f;
	}
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