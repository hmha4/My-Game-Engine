#include "Framework.h"
#include "SmoothFollow.h"

SmoothFollow::SmoothFollow(float moveSpeed, float rotationSpeed)
	: moveSpeed(moveSpeed), rotationSpeed(rotationSpeed)
	, num(0), distance(10.0f), lookAtPosition(0, 0, 0)
	, shaking(false), shakeMagnitude(0), shakeDuration(0), shakeTimer(0)
	, shakeOffset(0, 0, 0)
{
	velocity = D3DXVECTOR3(0, 0, 0);
	currentPos = D3DXVECTOR3(0, 0, 0);

	
}

SmoothFollow::~SmoothFollow()
{
}

void SmoothFollow::Update()
{
	//	Shake
	if (shaking == true)
	{
		//	��� �ð��� delta time���� ������
		shakeTimer += Time::Delta();

		//	����ð��� ���ӽð����� �������� ����� ����
		if (shakeTimer >= shakeDuration)
		{
			shaking = false;
			shakeTimer = shakeDuration;
		}

		// ����ð��� 0 ~ 1���̷� ���
		float progress = shakeTimer / shakeDuration;

		//	���� ������ ���������� �ٿ���
		//	�������� �پ���� �ʵ��� progress�� ������
		//	�������� �پ��� ������ �������� ������ ��
		float magnitude = shakeMagnitude * (1.0f - (progress * progress));

		//	���ο� offset�� -1, 1 ������ ������ ������ ����
		shakeOffset.x = Math::Random(-1.0f, 1.0f) * magnitude;
		shakeOffset.y = Math::Random(-1.0f, 1.0f) * magnitude;
		shakeOffset.z = Math::Random(-1.0f, 1.0f) * magnitude;

		//	ȸ���� ���� ��鸲
		D3DXVECTOR2 rotation;
		Rotation(&rotation);
		rotation.x += Math::ToRadian(shakeOffset.x);
		rotation.y += Math::ToRadian(shakeOffset.y);
		Rotation(rotation.x, rotation.y);
	}

	//	Translation
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

		//	+ shakeOffset : position�� ���� ��鸲
		Position(position.x + shakeOffset.x, position.y + shakeOffset.y, position.z + shakeOffset.z);
	}
}

void SmoothFollow::Shake(float magnitude, float duration)
{
	if (shaking == true) return;

	//	��鸲 On!
	shaking = true;

	//	���̶� �ð� ����
	shakeMagnitude = magnitude;
	shakeDuration = duration;

	//	��鸲 �߰� �ð� �ʱ�ȭ
	shakeTimer = 0.0f;
}
