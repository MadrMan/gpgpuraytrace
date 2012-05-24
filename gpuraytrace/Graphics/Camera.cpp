#include <Common.h>
#include "Camera.h"

#include "../Factories/IWindow.h"
#include "../Common/Timer.h"
#include "../Common/IInput.h"
#include "../Common/Logger.h"

Camera::Camera()
{
	position = XMVectorSet(0.0f, 50.0f, 0.0f, 0.0f);
	rotation = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	
	rotationEuler[0] = -3.0f;
	rotationEuler[1] = -4.6f;
	rotationEuler[2] = 0.0f;

	window = nullptr;

	nearZ = 0.05f;
	farZ = 5000.0f;

	kinectMove = 0.0f;
	kinectRotate = 0.0f;

	kinectFacade = new Facade();
	kinectFacade->AttachListener(this);
	kinectFacade->Start();
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

	kinectFacade->Stop();
	kinectFacade->DetachListener(this);
	delete kinectFacade;
}

void Camera::Update(float* data)
{
	kinectMove = data[2];
	kinectRotate = data[0];
	Logger() << kinectMove;
}

void Camera::setWindow(IWindow* window)
{
	this->window = window;

	const float FOV = XMConvertToRadians(75.0f);
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

		rotationEuler[1] += kinectRotate;
		
		//maximize up/down so camera control does not flip
		rotationEuler[0] = std::max(-XM_PI * 1.5f + 0.0001f, std::min(-XM_PIDIV2 - 0.0001f, rotationEuler[0]));
	}

	//XMVECTOR extraRotation = XMQuaternionRotationRollPitchYaw(rotateUD->getState() * 0.01f, rotateLR->getState() * 0.01f, 0.0f);
	//rotation = XMQuaternionMultiply(extraRotation, rotation);
	float moveSpeed = Timer::get()->getConstant() * 5.0f;

	//warpspeed
	if(warpDrive->getState() > 0.5f) moveSpeed *= 20.0f;

	float forward = moveForward->getState();
	forward += kinectMove * 5.0f;

	position = XMVectorAdd(position, front * forward * moveSpeed);
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