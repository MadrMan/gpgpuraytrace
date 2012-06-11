#pragma once

class __declspec(dllexport) DirectionMargin
{
public:
	DirectionMargin(void);
	~DirectionMargin(void);

	void Reset();

	float& operator[](const int index);

private:
	float m_margins[4];
	int m_size;
};