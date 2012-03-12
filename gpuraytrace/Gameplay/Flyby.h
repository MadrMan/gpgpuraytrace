#pragma once

class Camera;

class Flyby
{
public:
	Flyby(Camera* camera);
	virtual ~Flyby();

	void fly();

private:
	Camera* camera;
};