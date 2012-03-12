#pragma once

#include <vector>

namespace MouseButtons { enum T
{
	LeftButton = 0, 
	RightButton = 1, 
	MiddleButton = 2,
	Size
};}

namespace TriggerState { enum T
{
	Released,
	Holding,
	Held,
	Releasing
};}

namespace TriggerType { enum T
{
	OnTrigger,
	OnHold,
	OnRelease
};}

struct KeyboardTrigger
{
	int key;
	float value;
	TriggerType::T trigger;
};

struct MouseTrigger
{
	int key;
	float value;
	TriggerType::T trigger;
};

class IInputAction
{
public:
	virtual float getState() const = 0;
	virtual bool isTriggered() const = 0;

	virtual void registerKeyboard(int key, float highValue, TriggerType::T trigger = TriggerType::OnTrigger) = 0;
	virtual void registerMouseButton(MouseButtons::T button, float highValue, TriggerType::T trigger = TriggerType::OnTrigger) = 0;
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