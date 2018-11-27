#pragma once
#include "stdafx.h"
#include "../Draw/ILine.h"

struct Ray : public ILine
{
	Ray()
	{
		Position = Direction = D3DXVECTOR3(0, 0, 0);
	}
	
	Ray(D3DXVECTOR3& position, D3DXVECTOR3& direction)
		: Position(position), Direction(direction)
	{

	}

	void GetLine(D3DXMATRIX& world, vector<D3DXVECTOR3>& lines)
	{
		lines.push_back(Position);
		lines.push_back(Position + (Direction * Distance));
	}

	D3DXVECTOR3 Position;
	D3DXVECTOR3 Direction;
	float Distance = 10.0f;
};