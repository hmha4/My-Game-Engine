#pragma once
#include "Framework.h"

class ColliderRay : public ColliderElement
{
public:
	ColliderRay(D3DXVECTOR3& position, D3DXVECTOR3& direction)
	{
		localPosition = position;
		localDirection = direction;

		ray = new Ray(position, direction);

		Type((UINT)ColliderElement::EType::ERay);
	}
	ColliderRay(wstring name, D3DXVECTOR3& position, D3DXVECTOR3& direction)
	{
		localPosition = position;
		localDirection = direction;

		ray = new Ray(position, direction);

		Name(name);
		Type((UINT)ColliderElement::EType::ERay);
	}
	~ColliderRay()
	{
		SAFE_DELETE(ray);
	}

	Ray * GetRay() { return ray; }
	D3DXVECTOR3 LocalPosition() { return localPosition; }
	D3DXVECTOR3 LocalDirection() { return localDirection; }
	D3DXVECTOR3 WorldPosition() { return ray->Position; }
	D3DXVECTOR3 WorldDirection() { return ray->Direction; }

	void Transform(D3DXMATRIX& world)
	{
		D3DXVec3TransformCoord(&ray->Position, &localPosition, &world);
		D3DXVec3TransformNormal(&ray->Direction, &localDirection, &world);
		D3DXVec3Normalize(&ray->Direction, &ray->Direction);
	}


private:
	D3DXVECTOR3 localPosition;
	D3DXVECTOR3 localDirection;

	Ray * ray;
};