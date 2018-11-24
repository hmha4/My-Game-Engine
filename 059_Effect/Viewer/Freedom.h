#pragma once
#include "Camera.h"

class Freedom : public Camera
{
public:
	Freedom(float moveSpeed = 40.0f, float rotationSpeed = 5.5f);
	~Freedom();

	// Camera��(��) ���� ��ӵ�
	virtual void Update() override;

private:
	float moveSpeed;
	float rotationSpeed;
};