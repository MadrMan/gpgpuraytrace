#pragma once

#include "../IInput.h"
#include <vector>

struct KeyboardTrigger
{
	int key;
	float value;
};

struct MouseTrigger
{
	int key;
	float value;
};

class InputWinAPI;
class InputActionWinAPI : public IInputAction
{
public:
	InputActionWinAPI(InputWinAPI* input);
	virtual float getState() override;
	virtual void update() override;

	virtual void registerKeyboard(int key, float highValue) override;
	virtual void registerMouseButton(MouseButtons::T button, float highValue) override;
	virtual void registerMouseAxis(int axis) override;

private:
	float state;
	InputWinAPI* input;

	std::vector<KeyboardTrigger> keyboardKeys;
	std::vector<MouseTrigger> mouseButtons;
	std::vector<int> mouseAxis;
};

class InputWinAPI : public IInput
{
public:
	InputWinAPI();
	virtual ~InputWinAPI();

	virtual void update() override;
	bool handleWindowMessage(UINT Msg, WPARAM wParam, LPARAM lParam);

	virtual IInputAction* createAction() override;

private:
	friend class InputActionWinAPI;
	int mouseXPos;
	int mouseYPos;
	int keys[256];
	int mouseButtons[MouseButtons::Size];
	float mouseAxis[3];
};