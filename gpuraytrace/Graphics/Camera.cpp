#include <Common.h>
#include "Camera.h"

#include "../Factories/IWindow.h"
#include "../Common/Timer.h"
#include "IInput.h"
#include "Logger.h"

Camera::Camera()
{
	position = XMVectorSet(20.0f, 80.0f, 20.0f, 0.0f);
	rotation = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	
	rotationEuler[0] = 0.0f;
	rotationEuler[1] = 0.0f;
	rotationEuler[2] = 0.0f;
}

Camera::~Camera()
{
	input->destroyAction(moveSide);
	input->destroyAction(moveForward);
	input->destroyAction(rotateLR);
	input->destroyAction(rotateUD);
	input->destroyAction(disableMouse);
	input->destroyAction(warpDrive);
}

void Camera::setWindow(IWindow* window)
{
	matProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 
		(FLOAT)window->getWindowSettings().width / (FLOAT)window->getWindowSettings().height, 0.1f, 1000.0f);

	//Get input
	input = window->getInput();

	//Register camera input
	moveSide = window->getInput()->createAction();
	moveForward = window->getInput()->createAction();
	rotateLR = window->getInput()->createAction();
	rotateUD = window->getInput()->createAction();
	disableMouse = window->getInput()->createAction();
	warpDrive = window->getInput()->createAction();

	moveSide->registerKeyboard('A', -1.0f, TriggerType::OnHold);
	moveSide->registerKeyboard('D', 1.0f, TriggerType::OnHold);
	moveForward->registerKeyboard('W', 1.0f, TriggerType::OnHold);
	moveForward->registerKeyboard('S', -1.0f, TriggerType::OnHold);
	disableMouse->registerKeyboard(VK_CONTROL, 1.0f, TriggerType::OnHold);
	warpDrive->registerKeyboard('Q', 1.0f, TriggerType::OnHold);

	rotateLR->registerMouseAxis(0);
	rotateUD->registerMouseAxis(1);
}


void Camera::update()
{
	//if ctrl is hold down, discard mouse move
	if(disableMouse->getState() < 0.5f)
	{
		//Rotate camera
		rotationEuler[0] -= rotateUD->getState() * 0.01f;
		rotationEuler[1] -= rotateLR->getState() * 0.01f;
	}
	//XMVECTOR extraRotation = XMQuaternionRotationRollPitchYaw(rotateUD->getState() * 0.01f, rotateLR->getState() * 0.01f, 0.0f);
	rotation = XMQuaternionRotationRollPitchYaw(rotationEuler[0], rotationEuler[1], 0.0f);
	//rotation = XMQuaternionMultiply(extraRotation, rotation);

	float moveSpeed = Timer::get()->getConstant() * 5.0f;

	//warpspeed
	if(warpDrive->getState() > 0.5f)
	{
		moveSpeed *= 10.0f;
	}
	//Move camera
	XMVECTOR front = XMVector3Rotate(XM_FRONT, rotation);
	position = XMVectorAdd(position, front * moveForward->getState() * moveSpeed);
	XMVECTOR right = XMVectorSetY(XMVector3Rotate(front, XMQuaternionRotationRollPitchYaw(0.0f, -XM_PIDIV2, 0.0f)), 0.0f);
	position = XMVectorAdd(position, right * moveSide->getState() * moveSpeed);

	//Update matrices
	XMVECTOR direction = XMVector3Rotate(XM_FRONT, rotation);
	matView = XMMatrixLookToLH(position, direction, XM_UP);

	matViewProjection = XMMatrixMultiply(matView, matProjection);
}