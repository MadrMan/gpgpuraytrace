#pragma once

#include <vector>

namespace MouseButtons { enum T
{
	LeftButton = 0, 
	RightButton = 1, 
	MiddleButton = 2,
	Size
};}

class IInputAction
{
public:
	virtual float getState() = 0;

	virtual void registerKeyboard(int key, float highValue) = 0;
	virtual void registerMouseButton(MouseButtons::T button, float highValue) = 0;
	virtual void registerMouseAxis(int axis) = 0;

protected:
	IInputAction() { }
	virtual ~IInputAction() { }

private:
	friend class IInput;
	virtual void update() = 0;
};

class IInput
{
public:
	virtual ~IInput();

	virtual IInputAction* createAction() = 0;
	void destroyAction(IInputAction* action);
	
	virtual void update();

protected:
	IInput();
	std::vector<IInputAction*> actions;

};