#include "Framework.h"
#include "Orbit.h"

Orbit::Orbit(float moveSpeed, float rotationSpeed)
	: moveSpeed(moveSpeed), rotationSpeed(rotationSpeed)
	, num(0), distance(400.0f)
{

}

Orbit::~Orbit()
{
}

void Orbit::Update()
{
	//Rotation
	{
		D3DXVECTOR2 rotation;
		Rotation(&rotation);

		//if (Keyboard::Get()->Press('W'))
		//	rotation.x += rotationSpeed * Time::Delta();
		//
		//else if (Keyboard::Get()->Press('S'))
		//	rotation.x -= rotationSpeed * Time::Delta();
		//
		//if (Keyboard::Get()->Press('A'))
		//	rotation.y += rotationSpeed * Time::Delta();
		//
		//else if (Keyboard::Get()->Press('D'))
		//	rotation.y -= rotationSpeed * Time::Delta();
		//
		//Rotation(rotation.x, rotation.y);

		if (Mouse::Get()->Press(1))
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

		if (Keyboard::Get()->Press('W')) {
			distance -= moveSpeed * Time::Delta();
			if (distance <= 3.0f)
				distance = 3.0f;
		}
		else if (Keyboard::Get()->Press('S'))
			distance += moveSpeed * Time::Delta();
		
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

