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
	ColliderRay(Effect * effect, wstring name, D3DXVECTOR3& position, D3DXVECTOR3& direction, D3DXCOLOR& color = D3DXCOLOR(0, 1, 0, 1))
	{
		localPosition = position;
		localDirection = direction;

		ray = new Ray(position, direction);
		drawLine = new DebugLine(effect);
		drawLine->Initialize();
		drawLine->Ready();
		drawLine->Color(color);
		drawLine->Draw(__super::Transform(), ray);

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

	void Render()
	{
		if (IsDraw() == false) return;
		if (IsActive() == false) return;

		D3DXMATRIX result = root * transform;
		drawLine->Render(1, &result);
	}

	void Render(D3DXMATRIX val)
	{
		if (IsDraw() == false) return;
		if (IsActive() == false) return;

		D3DXMATRIX temp = root * val;
		drawLine->Render(1, &temp);
	}

	void Transform(D3DXMATRIX world) override
	{
		if (isActive == false)
			return;

		transform = world;

		D3DXMATRIX result = root * world;
		D3DXVec3TransformCoord(&ray->Position, &localPosition, &result);
		D3DXVec3TransformNormal(&ray->Direction, &localDirection, &result);
		D3DXVec3Normalize(&ray->Direction, &ray->Direction);
	}

private:
	D3DXVECTOR3 localPosition;
	D3DXVECTOR3 localDirection;

	Ray * ray;
};