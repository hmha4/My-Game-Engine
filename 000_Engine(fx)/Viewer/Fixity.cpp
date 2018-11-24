#include "stdafx.h"
#include "Fixity.h"

Fixity::Fixity()
{

}

Fixity::~Fixity()
{
}

void Fixity::Update()
{
	Rotation();
	View();
}

void Fixity::LookAtPosition(D3DXVECTOR3 * vec)
{
}

void Fixity::LookAtPosition(float x, float y, float z)
{
}
