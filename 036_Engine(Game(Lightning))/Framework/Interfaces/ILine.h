#pragma once
#include "Framework.h"

class ILine
{
public:
	ILine() {}
	virtual ~ILine() {}

	virtual void GetLine(D3DXMATRIX& world, vector<D3DXVECTOR3>& lines) = 0;
};