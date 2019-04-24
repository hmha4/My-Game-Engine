#include "Framework.h"
#include "SmoothFollow.h"

SmoothFollow::SmoothFollow(float moveSpeed, float rotationSpeed)
	: moveSpeed(moveSpeed), rotationSpeed(rotationSpeed)
	, num(0), distance(10.0f), lookAtPosition(0, 0, 0)
{
	velocity = D3DXVECTOR3(0, 0, 0);
	currentPos = D3DXVECTOR3(0, 0, 0);
}

SmoothFollow::~SmoothFollow()
{
}

void SmoothFollow::Update()
{
	//Rotation
	{
		//D3DXVECTOR2 currRot;
		//__super::Rotation(&currRot);
		//currentRot = D3DXVECTOR3(currRot.x, currRot.y, 0);
		//
		//D3DXVECTOR2 rotation;
		//rotationTime += rotationSpeed * Time::Delta();
		//
		//if (rotationTime <= 1.0f)
		//{
		//	Math::SmootherStep(rotation, currentRot, targetRot, rotationTime);
		//}
		//else
		//	rotationTime = 0.0f;
		//
		//
		//
		//__super::Rotation(rotation.x, rotation.y);

		//if (Mouse::Get()->Press(1))
		//{
		//	D3DXVECTOR2 rotation;
		//	__super::Rotation(&rotation);
		//	D3DXVECTOR3 val = Mouse::Get()->GetMoveValue();
		//
		//	rotation.x += val.y * 6.5f * Time::Delta();
		//	rotation.y += val.x * 6.5f * Time::Delta();
		//
		//	__super::Rotation(rotation.x, rotation.y);
		//}
	}

	//Translation
	{
		D3DXVECTOR3 position(0, 0, 0);

		float val = Mouse::Get()->GetMoveValue().z;

		distance += -val * 5 * Time::Delta();
		if (distance <= 1.0f)
			distance = 0.5f;

		D3DXMATRIX matRotation;
		MatrixRotation(&matRotation);

		position.z = -distance;
		D3DXVec3TransformCoord(&position, &position, &matRotation);

		currentPos = Math::SmoothDamp(currentPos, lookAtPosition, velocity, 0.3f);

		position.x += currentPos.x;
		position.y += currentPos.y;
		position.z += currentPos.z;

		Position(position.x, position.y, position.z);
	}
}