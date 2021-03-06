#include <Common.h>
#include "Camera.h"

#include "../Factories/IWindow.h"
#include "../Common/Timer.h"
#include "../Common/IInput.h"
#include "../Common/Logger.h"

const XMVECTOR Camera::initialPosition = XMVectorSet(0.0f, 100.0f, 0.0f, 0.0f);
const float Camera::initialRotationEuler[3]= {-3.0f,-4.6f,0.0f};

Camera::Camera()
{
	rotation = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	resetPosition();

	window = nullptr;

	nearZ = 0.01f;
	farZ = 5000.0f;
}

Camera::~Camera()
{
	input->destroyAction(moveSide);
	input->destroyAction(moveForward);
	input->destroyAction(rotateLR);
	input->destroyAction(rotateUD);
	input->destroyAction(disableMouse);
	input->destroyAction(warpDrive);
	input->destroyAction(moveUp);
	input->destroyAction(moveDown);
}

void Camera::setWindow(IWindow* window)
{
	this->window = window;

	const float FOV = XMConvertToRadians(80.0f);
	const float ASPECT = (FLOAT)window->getWindowSettings().width / (FLOAT)window->getWindowSettings().height;

	matProjection = XMMatrixPerspectiveFovLH(FOV, ASPECT, nearZ, farZ);

	//Get input
	input = window->getInput();

	//Register camera input
	moveSide = window->getInput()->createAction();
	moveForward = window->getInput()->createAction();
	rotateLR = window->getInput()->createAction();
	rotateUD = window->getInput()->createAction();
	disableMouse = window->getInput()->createAction();
	warpDrive = window->getInput()->createAction();
	moveDown = window->getInput()->createAction();
	moveUp = window->getInput()->createAction();

	moveSide->registerKeyboard('A', -1.0f, TriggerType::OnHold);
	moveSide->registerKeyboard('D', 1.0f, TriggerType::OnHold);
	moveForward->registerKeyboard('W', 1.0f, TriggerType::OnHold);
	moveForward->registerMouseButton(MouseButtons::LeftButton, 1.0f, TriggerType::OnHold);
	moveForward->registerMouseButton(MouseButtons::RightButton, -1.0f, TriggerType::OnHold);
	moveForward->registerKeyboard('S', -1.0f, TriggerType::OnHold);
	disableMouse->registerKeyboard(VK_CONTROL, 1.0f, TriggerType::OnHold);
	warpDrive->registerKeyboard(VK_SHIFT, 1.0f, TriggerType::OnHold);
	moveUp->registerKeyboard('Q', 1.0f, TriggerType::OnHold);
	moveDown->registerKeyboard('E', 1.0f, TriggerType::OnHold);

	rotateLR->registerMouseAxis(0);
	rotateUD->registerMouseAxis(1);
}

void Camera::move()
{
	//if ctrl is hold down, discard mouse move
	if(disableMouse->getState() < 0.5f)
	{
		//Rotate camera
		rotationEuler[0] -= rotateUD->getState() * 0.01f;
		rotationEuler[1] -= rotateLR->getState() * 0.01f;
		
		//maximize up/down so camera control does not flip
		rotationEuler[0] = std::max(-XM_PI * 1.5f + 0.0001f, std::min(-XM_PIDIV2 - 0.0001f, rotationEuler[0]));
	}

	//XMVECTOR extraRotation = XMQuaternionRotationRollPitchYaw(rotateUD->getState() * 0.01f, rotateLR->getState() * 0.01f, 0.0f);
	//rotation = XMQuaternionMultiply(extraRotation, rotation);
	float moveSpeed = Timer::get()->getConstant() * 5.0f;

	//warpspeed
	if(warpDrive->getState() > 0.5f) moveSpeed *= 5.0f;

	position = XMVectorAdd(position, front * moveForward->getState() * moveSpeed);
	if(moveUp->getState()) position = XMVectorAdd(position, XMVectorSet(0.0f, moveSpeed, 0.0f, 0.0f));
	if(moveDown->getState()) position = XMVectorSubtract(position, XMVectorSet(0.0f, moveSpeed, 0.0f, 0.0f));

	XMVECTOR right = XMVectorSetY(XMVector3Rotate(front, XMQuaternionRotationRollPitchYaw(0.0f, -XM_PIDIV2, 0.0f)), 0.0f);
	right = XMVector3Normalize(right);
	position = XMVectorAdd(position, right * moveSide->getState() * moveSpeed);
}

void Camera::rotate()
{
	//Move camera
	rotation = XMQuaternionRotationRollPitchYaw(rotationEuler[0], rotationEuler[1], 0.0f);
	front = XMVector3Rotate(XM_FRONT, rotation);
}

void Camera::update()
{
	//Update matrices
	matView = XMMatrixLookToLH(position, front, XM_UP);
	matViewProjection = XMMatrixMultiply(matView, matProjection);
}

void Camera::resetPosition()
{
	position = initialPosition; 
	rotationEuler[0] = initialRotationEuler[0];
	rotationEuler[1] = initialRotationEuler[1];
	rotationEuler[2] = initialRotationEuler[2];
}