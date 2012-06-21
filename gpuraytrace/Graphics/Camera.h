#pragma once

#include "../Common/Types.h"

class IWindow;
class IInputAction;
class IInput;

static const XMVECTOR XM_UP = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
static const XMVECTOR XM_FRONT = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

//! Camera class which is used to navigate trough the 3D scene
CLASSALIGN class Camera
{
public:
	ALLOCALIGN;

	//! Constructor
	Camera();

	//! Destructor
	virtual ~Camera();

	//! Set the window for this camera, so the perspective and input handling can be set up
	void setWindow(IWindow* window);

	//! Get the window set for this camera
	IWindow* getWindow() const
	{ return window; }

	//! Move the camera one frame based on set movement
	void move();

	//! Update the camera movement based on the current input
	void update();

	//! Update the rotation based on movement
	void rotate();

	//! Get the near Z plane distance
	float getNearZ() const
	{ return nearZ; }

	//! Get the far Z plane distance
	float getFarZ() const
	{ return farZ; }

	//! Move te camera to start position
	void resetPosition();

	XMVECTOR position;
	XMVECTOR front;
	XMMATRIX matProjection;
	XMMATRIX matView;
	float rotationEuler[3];

private:
	static const XMVECTOR initialPosition;
	static const float initialRotationEuler[];

	XMVECTOR rotation;
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
	float nearZ, farZ;
};