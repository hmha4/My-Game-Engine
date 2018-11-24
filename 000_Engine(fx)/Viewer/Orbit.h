#pragma once
#include "Camera.h"

class Orbit : public Camera
{
public:
	Orbit(float moveSpeed = 40.0f, float rotationSpeed = 6.5f);
	~Orbit();

	// Camera을(를) 통해 상속됨
	virtual void Update() override;

	void LookAtPosition(D3DXVECTOR3* vec)
	{
		*vec = lookAtPosition;
	}

	void LookAtPosition(float x, float y, float z)
	{
		lookAtPosition = D3DXVECTOR3(x, y, z);
	}
private:
	float moveSpeed;
	float rotationSpeed;

	int num;
	float distance;

	D3DXVECTOR3 lookAtPosition;
};