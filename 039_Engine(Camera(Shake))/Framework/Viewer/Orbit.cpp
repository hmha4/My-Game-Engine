#include "Framework.h"
#include "Orbit.h"

Orbit::Orbit(float moveSpeed, float rotationSpeed)
	: moveSpeed(moveSpeed), rotationSpeed(rotationSpeed)
	, num(0), distance(10.0f)
{
}

Orbit::~Orbit()
{
}

void Orbit::Update()
{
	if (Mouse::Get()->Press(2))
	{
		D3DXVECTOR3 pos;
		LookAtPosition(&pos);

		D3DXVECTOR3 val = Mouse::Get()->GetMoveValue();

		pos.x += -val.x * moveSpeed * Time::Delta();
		pos.y += val.y * moveSpeed * Time::Delta();

		LookAtPosition(pos.x, pos.y, pos.z);
	}

	//Rotation
	{
		D3DXVECTOR2 rotation;
		Rotation(&rotation);

		if (Keyboard::Get()->Press(VK_LCONTROL) && Mouse::Get()->Press(1))
		{
			D3DXVECTOR3 val = Mouse::Get()->GetMoveValue();

			rotation.x += val.y * rotationSpeed * Time::Delta();
			rotation.y += val.x * rotationSpeed * Time::Delta();

			Rotation(rotation.x, rotation.y);
		}
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

		position.x += lookAtPosition.x;
		position.y += lookAtPosition.y;
		position.z += lookAtPosition.z;

		Position(position.x, position.y, position.z);
	}
}