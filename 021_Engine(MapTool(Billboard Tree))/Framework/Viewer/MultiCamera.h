#pragma once
#include "Camera.h"

class MultiCamera : public Camera
{
public:
	MultiCamera(wstring camType, float moveSpeed = 40.0f, float rotationSpeed = 6.5f);
	~MultiCamera();

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

	void CameraType(wstring camType) { this->camType = camType; }
	wstring CameraType() { return camType; }
	void Speed(float val) { moveSpeed = val; }
	float Speed() { return moveSpeed; }
private:
	wstring camType;

	float moveSpeed;
	float rotationSpeed;

	int num;
	float distance;

	D3DXVECTOR3 lookAtPosition;
};