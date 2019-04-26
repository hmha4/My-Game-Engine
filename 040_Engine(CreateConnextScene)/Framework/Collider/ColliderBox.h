#pragma once
#include "Framework.h"

class ColliderBox : public ColliderElement
{
public:
	ColliderBox(D3DXVECTOR3& min, D3DXVECTOR3& max)
		: ColliderElement()
	{
		localMin = min;
		localMax = max;

		boundingBox = new BBox(min, max);
		
		Type((UINT)ColliderElement::EType::EBox);
	}
	ColliderBox(wstring name, D3DXVECTOR3& min, D3DXVECTOR3& max)
		: ColliderElement()
	{
		localMin = min;
		localMax = max;

		boundingBox = new BBox(min, max);

		Name(name);
		Type((UINT)ColliderElement::EType::EBox);
	}
	ColliderBox(Effect * effect, wstring name, D3DXVECTOR3& min, D3DXVECTOR3& max, D3DXCOLOR& color = D3DXCOLOR(0, 1, 0, 1))
		: ColliderElement()
	{
		localMin = min;
		localMax = max;

		boundingBox = new BBox(min, max);
		drawLine = new DebugLine(effect);
		drawLine->Initialize();
		drawLine->Ready();
		drawLine->Color(color);
		drawLine->Draw(__super::Transform(), boundingBox);

		Name(name);
		Type((UINT)ColliderElement::EType::EBox);
	}
	~ColliderBox()
	{
		SAFE_DELETE(boundingBox);
	}

	D3DXVECTOR3 LocalMin() { return localMin; }
	D3DXVECTOR3 LocalMax() { return localMax; }
	D3DXVECTOR3 WorldMin() { return boundingBox->Min; }
	D3DXVECTOR3 WorldMax() { return boundingBox->Max; }

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

		D3DXMATRIX result = root * transform;
		D3DXVec3TransformCoord(&boundingBox->Min, &localMin, &result);
		D3DXVec3TransformCoord(&boundingBox->Max, &localMax, &result);
	}

	BBox * BoundingBox() { return boundingBox; }
private:
	D3DXVECTOR3 localMin;
	D3DXVECTOR3 localMax;

	BBox * boundingBox;
};