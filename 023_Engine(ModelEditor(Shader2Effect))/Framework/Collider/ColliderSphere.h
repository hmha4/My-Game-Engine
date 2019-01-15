#pragma once
#include "Framework.h"

class ColliderSphere : public ColliderElement
{
public:
	ColliderSphere(wstring name, D3DXVECTOR3 center, float radius)
		: ColliderElement()
	{
		localCenter = center;

		boundingSphere = new BSphere(center, radius);

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

	void Transform(D3DXMATRIX val)
	{
		D3DXVec3TransformCoord(&boundingSphere->Center, &localCenter, &val);

		float x = val._11;
		float y = val._22;
		float z = val._33;

		float temp = max(x, max(y, z));
		boundingSphere->Radius *= temp;
	}
private:
	D3DXVECTOR3 localCenter;
	BSphere * boundingSphere;
};