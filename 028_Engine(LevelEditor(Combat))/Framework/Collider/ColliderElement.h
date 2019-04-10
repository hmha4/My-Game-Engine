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
	virtual ~ColliderElement();

	virtual void Render() {}
	virtual void Render(D3DXMATRIX val) {}

	// ------------------------------------------------------------------------- //
	//	Get/Set transform matrix.
	// ------------------------------------------------------------------------- //
	virtual void Transform(D3DXMATRIX val) { transform = val; }
	virtual D3DXMATRIX Transform() { return transform; }
	void Root(D3DXMATRIX& val) { root = val; }
	D3DXMATRIX Root() { return root; }
	void BoneIndex(int val) { boneIndex = val; }
	int BoneIndex() { return boneIndex; }

	// ------------------------------------------------------------------------- //
	//	Get/Set Parent layer
	// ------------------------------------------------------------------------- //
	void ParentLayer(CollisionLayer * layer) { parentLayer = layer; }
	CollisionLayer * ParentLayer() { return parentLayer; }

	// ------------------------------------------------------------------------- //
	//	Removes this element in the collision layer.
	// ------------------------------------------------------------------------- //
	void RemoveInLayer();

	// ------------------------------------------------------------------------- //
	//	Draw Line
	// ------------------------------------------------------------------------- //
	void IsDraw(bool val) { isDraw = val; }
	bool IsDraw() { return isDraw; }
	virtual void IsActive(bool val) { isActive = val; }
	virtual bool IsActive() { return isActive; }
	void SetColor(float x, float y, float z);
	void SetColor(D3DXCOLOR &color);
protected:
	wstring name;
	UINT type;

	D3DXMATRIX root;
	D3DXMATRIX transform;

	CollisionLayer * parentLayer;

	bool isDraw;
	bool isActive;
	class DebugLine * drawLine;

	int boneIndex;
private:

public:
	void Name(wstring val);
	wstring Name();

	// IType을(를) 통해 상속됨
	void Type(UINT val) override;
	UINT Type() override;
};