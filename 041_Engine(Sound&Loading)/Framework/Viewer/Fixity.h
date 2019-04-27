#pragma once
#include "Camera.h"

class Fixity : public Camera
{
public:
	Fixity();
	~Fixity();

	void Update();

private:

	// Camera을(를) 통해 상속됨
	virtual void LookAtPosition(D3DXVECTOR3 * vec) override;

	virtual void LookAtPosition(float x, float y, float z) override;
};