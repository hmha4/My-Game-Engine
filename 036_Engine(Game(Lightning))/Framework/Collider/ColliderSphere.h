#pragma once
#include "Framework.h"

class ColliderSphere : public ColliderElement
{
public:
	ColliderSphere(D3DXVECTOR3 center, float radius)
		: ColliderElement()
	{
		localCenter = center;

		boundingSphere = new BSphere(center, radius);

		Type((UINT)ColliderElement::EType::ESphere);
	}
	ColliderSphere(wstring name, D3DXVECTOR3 center, float radius)
		: ColliderElement()
	{
		localCenter = center;

		boundingSphere = new BSphere(center, radius);

		Name(name);
		Type((UINT)ColliderElement::EType::ESphere);
	}
	ColliderSphere(Effect * effect, wstring name, D3DXVECTOR3 center, float radius, D3DXCOLOR& color = D3DXCOLOR(0, 1, 0, 1))
		: ColliderElement()
	{
		localCenter = center;

		boundingSphere = new BSphere(center, radius);
		drawLine = new DebugLine(effect);
		drawLine->Initialize();
		drawLine->Ready();
		drawLine->Color(0, 1, 0);
		drawLine->Draw(__super::Transform(), boundingSphere);

		Name(name);
		Type((UINT)ColliderElement::EType::ESphere);
	}
	~ColliderSphere()
	{
		SAFE_DELETE(boundingSphere);
	}

	BSphere * BoundingSphere() { return boundingSphere; }

	D3DXVECTOR3 LocalCenter() { return localCenter; }
	D3DXVECTOR3 WorldCenter() { return boundingSphere->Center; }

	float Radius() { return boundingSphere->Radius; }

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

	void Transform(D3DXMATRIX val)
	{
		if (isActive == false)
			return;

		transform = val;

		D3DXMATRIX result = root * transform;
		D3DXVec3TransformCoord(&boundingSphere->Center, &localCenter, &result);

		float x = result._11;
		float y = result._22;
		float z = result._33;

		float temp = max(x, max(y, z));
		boundingSphere->Radius = temp;
	}

	D3DXMATRIX Transform()
	{
		return transform;
	}
private:
	D3DXVECTOR3 localCenter;
	BSphere * boundingSphere;
};