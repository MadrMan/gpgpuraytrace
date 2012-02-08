#include "Common.h"
#include "InputWinAPI.h"

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

void InputActionWinAPI::registerMouseButton(int button, float highValue)
{
	MouseTrigger mt = {button, highValue};
	mouseButtons.push_back(mt);
}

void InputActionWinAPI::registerMouseAxis(int axis)
{
	mouseAxis.push_back(axis);
}

InputWinAPI::InputWinAPI()
{

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
	wParam;lParam;

	switch(Msg)
	{
	case WM_KEYDOWN:
		return true;
	case WM_KEYUP:
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