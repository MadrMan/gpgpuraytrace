#include <Common.h>
#include "Camera.h"

#include "../Factories/IWindow.h"

Camera::Camera()
{
	position = XMVectorSet(20.0f, 20.0f, 20.0f, 0.0f);
	rotation = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	
}

Camera::~Camera()
{

}

void Camera::setWindow(IWindow* window)
{
	matProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 
		(FLOAT)window->getWindowSettings().width / (FLOAT)window->getWindowSettings().height, 0.1f, 1000.0f);
}

void Camera::update()
{
	XMVECTOR direction = XMVector3Rotate(XM_FRONT, rotation);
	matView = XMMatrixLookToLH(position, direction, XM_UP);

	matViewProjection = XMMatrixMultiply(matView, matProjection);
}