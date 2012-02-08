#include "Common.h"
#include "IInput.h"

#include "InputWinAPI.h"

IInput::IInput()
{
}

IInput::~IInput()
{
}

void IInput::destroyAction(IInputAction* action)
{
	for(auto it = actions.begin(); it != actions.end(); it++)
	{
		if(*it == action)
		{
			actions.erase(it);
			delete action;
			return;
		}
	}
}

void IInput::update()
{
	for(auto it = actions.begin(); it != actions.end(); it++)
	{
		(*it)->update();
	}
}