#pragma once

#include <xnamath.h>

class IWindow;

//! Camera class which is used to navigate trough the 3D scene
class Camera
{
public:
	Camera();
	virtual ~Camera();

	void setWindow(IWindow* window);

	void update();
private:
	XMVECTOR position;
	XMVECTOR rotation;
	XMMATRIX matView;
	XMMATRIX matProjection;
	XMMATRIX matViewProjection;
};