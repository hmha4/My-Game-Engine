#pragma once
#include "Camera.h"

class SmoothFollow : public Camera
{
public:
	SmoothFollow(float moveSpeed = 40.0f, float rotationSpeed = 6.5f);
	~SmoothFollow();

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

	//void Rotation(float x, float y) override
	//{
	//	targetRot = D3DXVECTOR2(x, y);
	//}
	//void Position(float x, float y, float z) override
	//{
	//	targetPos = D3DXVECTOR3(x, y, z);
	//}

	void Speed(float val) { moveSpeed = val; }
	float Speed() { return moveSpeed; }
private:
	float moveSpeed;
	float rotationSpeed;

	int num;
	float distance;

	D3DXVECTOR3 lookAtPosition;

	float rotationTime;
	D3DXVECTOR2 currentRot;
	D3DXVECTOR2 targetRot;

	float moveTime;
	D3DXVECTOR3 currentPos;
	D3DXVECTOR3 targetPos;

	D3DXVECTOR3 velocity;
};