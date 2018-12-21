#pragma once
#include "Camera.h"

class Freedom : public Camera
{
public:
	Freedom(float moveSpeed = 40.0f, float rotationSpeed = 5.5f);
	~Freedom();

	// Camera을(를) 통해 상속됨
	virtual void Update() override;

	void MoveSpeed(float val) { moveSpeed = val; }
	void RotationSpeed(float val) { rotationSpeed = val; }
private:
	float moveSpeed;
	float rotationSpeed;
};