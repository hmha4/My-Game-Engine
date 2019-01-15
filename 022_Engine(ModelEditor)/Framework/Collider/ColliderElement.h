#pragma once

class CollisionLayer;

#include "IType.h"
// ------------------------------------------------------------------------- //
//	It's a basic element of collision.
// ------------------------------------------------------------------------- //
class ColliderElement : public IType
{
public:
	enum class EType
	{
		EBox,
		ESphere,
		ERay,
		EModel,
	};
public:
	ColliderElement();
	~ColliderElement();

	// ------------------------------------------------------------------------- //
	//	Get/Set transform matrix.
	// ------------------------------------------------------------------------- //
	virtual void Transform(D3DXMATRIX val) { transform = val; }
	D3DXMATRIX Transform() { return transform; }

	// ------------------------------------------------------------------------- //
	//	Get/Set Parent layer
	// ------------------------------------------------------------------------- //
	void ParentLayer(CollisionLayer * layer) { parentLayer = layer; }
	CollisionLayer * ParentLayer() { return parentLayer; }

	// ------------------------------------------------------------------------- //
	//	Removes this element in the collison layer.
	// ------------------------------------------------------------------------- //
	void RemoveInLayer();

protected:
	wstring name;
	UINT type;

	D3DXMATRIX transform;

	CollisionLayer * parentLayer;

private:


public:
	void Name(wstring val);
	wstring Name();

	// IType을(를) 통해 상속됨
	void Type(UINT val) override;
	UINT Type() override;
};