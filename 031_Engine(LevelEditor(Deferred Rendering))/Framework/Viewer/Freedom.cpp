#include "Framework.h"
#include "Freedom.h"

Freedom::Freedom(float moveSpeed, float rotationSpeed)
	: moveSpeed(moveSpeed), rotationSpeed(rotationSpeed)
{
}

Freedom::~Freedom()
{
}

void Freedom::Update()
{
	//	회전된 방향으로 이동해야 하기 때문에 갱신된
	//	forward, right, up의 정보를 Camera로 부터 받아온다.
	D3DXVECTOR3 forward, right, up;
	Forward(&forward);
	Right(&right);
	Up(&up);

	//	이동 처리
	{
		D3DXVECTOR3 position;
		Position(&position);

		if (Keyboard::Get()->Press('W'))
			position += forward * moveSpeed * Time::Delta();
		else if (Keyboard::Get()->Press('S'))
			position += -forward * moveSpeed * Time::Delta();

		if (Keyboard::Get()->Press('A'))
			position += -right * moveSpeed * Time::Delta();
		else if (Keyboard::Get()->Press('D'))
			position += right * moveSpeed * Time::Delta();

		if (Keyboard::Get()->Press('E'))
			position += up * moveSpeed * Time::Delta();
		else if (Keyboard::Get()->Press('Q'))
			position += -up * moveSpeed * Time::Delta();

		Position(position.x, position.y, position.z);
	}

	//	회전 처리
	{
		D3DXVECTOR2 rotation;
		Rotation(&rotation);

		if (Mouse::Get()->Press(1))
		{
			D3DXVECTOR3 val = Mouse::Get()->GetMoveValue();

			rotation.x += val.y * rotationSpeed * Time::Delta();
			rotation.y += val.x * rotationSpeed * Time::Delta();

			Rotation(rotation.x, rotation.y);
		}
	}
}