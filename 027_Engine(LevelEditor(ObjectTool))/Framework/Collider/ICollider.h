#pragma once
#include "Framework.h"

class ICollider
{
public:
	virtual void CreateCollider(UINT type = 0) = 0;
	virtual vector<ColliderElement*> &GetCollider() = 0;
};