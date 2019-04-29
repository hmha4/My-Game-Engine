#include "Framework.h"
#include "ColliderElement.h"
#include "CollisionContext.h"

ColliderElement::ColliderElement()
	: name(L""), type(-1), parentLayer(NULL)
	, isDraw(false), boneIndex(-1)
	, isActive(true)
{
	D3DXMatrixIdentity(&root);
	D3DXMatrixIdentity(&transform);
}

ColliderElement::~ColliderElement()
{
	SAFE_DELETE(drawLine)
}

void ColliderElement::RemoveInLayer()
{
	if (parentLayer != NULL)
	{
		parentLayer->RemoveCollider(this);
		parentLayer = NULL;
	}
}

void ColliderElement::SetColor(float x, float y, float z)
{
	drawLine->Color(D3DXCOLOR(x, y, z, 1));
}

void ColliderElement::SetColor(D3DXCOLOR & color)
{
	drawLine->Color(color);
}

void ColliderElement::Name(wstring val)
{
	name = val;
}

wstring ColliderElement::Name()
{
	return name;
}

void ColliderElement::Type(UINT val)
{
	type = val;
}

UINT ColliderElement::Type()
{
	return type;
}