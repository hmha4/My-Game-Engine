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
		//	경과 시간을 delta time으로 더해줌
		shakeTimer += Time::Delta();

		//	경과시간이 지속시간보다 높아지면 흔들지 않음
		if (shakeTimer >= shakeDuration)
		{
			shaking = false;
			shakeTimer = shakeDuration;
		}

		// 경과시간을 0 ~ 1사이로 계산
		float progress = shakeTimer / shakeDuration;

		//	힘의 강도를 점진적으로 줄여줌
		//	선형으로 줄어들지 않도록 progress를 제곱함
		//	선형으로 줄어들고 싶으면 제곱하지 않으면 됨
		float magnitude = shakeMagnitude * (1.0f - (progress * progress));

		//	새로운 offset을 -1, 1 사이의 난수와 힘으로 생성
		shakeOffset.x = Math::Random(-1.0f, 1.0f) * magnitude;
		shakeOffset.y = Math::Random(-1.0f, 1.0f) * magnitude;
		shakeOffset.z = Math::Random(-1.0f, 1.0f) * magnitude;

		//	회전에 대한 흔들림
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

		//	+ shakeOffset : position에 대한 흔들림
		Position(position.x + shakeOffset.x, position.y + shakeOffset.y, position.z + shakeOffset.z);
	}
}

void SmoothFollow::Shake(float magnitude, float duration)
{
	if (shaking == true) return;

	//	흔들림 On!
	shaking = true;

	//	힘이랑 시간 설정
	shakeMagnitude = magnitude;
	shakeDuration = duration;

	//	흔들림 견과 시간 초기화
	shakeTimer = 0.0f;
}
