#pragma once

#include "../Common/Types.h"

#include <facade.h>

class IWindow;
class IInputAction;
class IInput;

static const XMVECTOR XM_UP = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
static const XMVECTOR XM_FRONT = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

//! Camera class which is used to navigate trough the 3D scene
CLASSALIGN class Camera : public IObserver<float*>
{
public:
	ALLOCALIGN;

	Camera();
	virtual ~Camera();

	void setWindow(IWindow* window);

	IWindow* getWindow() const
	{ return window; }

	void setInput(IInput* input);

	void move();
	void update();
	void rotate();

	float getNearZ() const
	{ return nearZ; }

	float getFarZ() const
	{ return farZ; }

	virtual void Update(float* data) override;

//private:
	XMVECTOR position;
	XMVECTOR rotation;
	XMVECTOR front;
	XMMATRIX matView;
	XMMATRIX matProjection;
	XMMATRIX matViewProjection;

	IInputAction* moveSide;
	IInputAction* moveForward;
	IInputAction* rotateLR;
	IInputAction* rotateUD;
	IInputAction* disableMouse;
	IInputAction* warpDrive;
	IInputAction* moveUp;
	IInputAction* moveDown;
	IInput* input;

	IWindow* window;
	Facade* kinectFacade;
	float kinectMove;
	float kinectRotate;

	float rotationEuler[3];
	float nearZ, farZ;
};