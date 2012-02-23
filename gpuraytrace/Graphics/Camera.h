#pragma once

#include "../Common/Types.h"
#include <xnamath.h>

class IWindow;

static const XMVECTOR XM_UP = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
static const XMVECTOR XM_FRONT = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

//! Camera class which is used to navigate trough the 3D scene
CLASSALIGN class Camera
{
public:
	ALLOCALIGN;

	Camera();
	virtual ~Camera();

	void setWindow(IWindow* window);

	void update();

//private:
	XMVECTOR position;
	XMVECTOR rotation;
	XMMATRIX matView;
	XMMATRIX matProjection;
	XMMATRIX matViewProjection;
};