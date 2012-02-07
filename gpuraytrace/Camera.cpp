#include "Common.h"
#include "Camera.h"

#include "IWindow.h"

Camera::Camera()
{
	position = XMVectorSet(20.0f, 20.0f, 20.0f, 0.0f);
	rotation = XMVectorZero();
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
	const XMVECTOR XM_UP = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR direction = XMVectorSet(-0.5f, -0.5f, -0.5f, 0.0f);
	matView = XMMatrixLookToLH(position, direction, XM_UP);

	matViewProjection = XMMatrixMultiply(matView, matProjection);
}