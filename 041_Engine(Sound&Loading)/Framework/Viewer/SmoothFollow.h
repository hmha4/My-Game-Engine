#pragma once
#include "Camera.h"

class SmoothFollow : public Camera
{
public:
	SmoothFollow(float moveSpeed = 40.0f, float rotationSpeed = 6.5f);
	~SmoothFollow();

	// Camera��(��) ���� ��ӵ�
	virtual void Update() override;

	void LookAtPosition(D3DXVECTOR3* vec)
	{
		*vec = lookAtPosition;
	}

	void LookAtPosition(float x, float y, float z)
	{
		lookAtPosition = D3DXVECTOR3(x, y, z);
	}

	void Speed(float val) { moveSpeed = val; }
	float Speed() { return moveSpeed; }

	void Shake(float magnitude, float duration);
	void SetDistance(float val) { distance = val; }
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

private:
	bool shaking;				//	��鸮�� �ִ°�?
	float shakeMagnitude;		//	��鸲�� �ִ� ��
	float shakeDuration;		//	��鸲�� ���� �ð�
	float shakeTimer;			//	��鸲�� ��� �ð�
	D3DXVECTOR3 shakeOffset;	//	��鸲 offset
};