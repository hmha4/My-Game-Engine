#include "Framework.h"
#include "ColliderElement.h"
#include "CollisionContext.h"

ColliderElement::ColliderElement()
	: name(L""), type(-1), parentLayer(NULL)
{
	D3DXMatrixIdentity(&transform);
}

ColliderElement::~ColliderElement()
{
}

void ColliderElement::RemoveInLayer()
{
	if (parentLayer != NULL)
	{
		parentLayer->RemoveCollider(this);
		parentLayer = NULL;
	}
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
