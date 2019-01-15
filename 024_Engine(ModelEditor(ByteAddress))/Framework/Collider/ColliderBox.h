#pragma once
#include "Framework.h"

class ColliderBox : public ColliderElement
{
public:
	ColliderBox(wstring name, D3DXVECTOR3& min, D3DXVECTOR3& max)
		: ColliderElement()
	{
		localMin = min;
		localMax = max;

		boundingBox = new BBox(min, max);

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

	void Transform(D3DXMATRIX& world)
	{
		D3DXVECTOR3 s, t;
		D3DXQUATERNION q;
		D3DXMatrixDecompose(&s, &q, &t, &world);

		D3DXMATRIX S, T, Result;

		D3DXMatrixScaling(&S, s.x, s.y, s.z);
		D3DXMatrixTranslation(&T, t.x, t.y, t.z);

		Result = S * T;

		D3DXVec3TransformCoord(&boundingBox->Min, &localMin, &Result);
		D3DXVec3TransformCoord(&boundingBox->Max, &localMax, &Result);
	}

	BBox * BoundingBox() { return boundingBox; }
private:
	D3DXVECTOR3 localMin;
	D3DXVECTOR3 localMax;

	BBox * boundingBox;
};