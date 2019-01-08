#include "Framework.h"
#include "GameRender.h"

GameRender::GameRender()
	: enable(true), visible(true)
	, isChild(false), parentName(L""), parentBone(NULL)
	, position(0, 0, 0), scale(1, 1, 1), rotation(0, 0, 0)
	, forward(0, 0, 1), up(0, 1, 0), right(1, 0, 0)
{
	D3DXMatrixIdentity(&parentBoneWorld);
	D3DXMatrixIdentity(&rootAxis);
	D3DXMatrixIdentity(&world);
}

GameRender::~GameRender()
{

}

void GameRender::Enable(bool val)
{
	enable = val;

	for (auto temp : Enabled)
		temp(val);
}

bool GameRender::Enable()
{
	return enable;
}

void GameRender::Visible(bool val)
{
	visible = val;

	for (auto temp : Visibled)
		temp(val);
}

bool GameRender::Visible()
{
	return visible;
}

void GameRender::RootAxis(D3DXMATRIX & matrix)
{
	rootAxis = matrix;
}

void GameRender::RootAxis(D3DXVECTOR3 & rotation)
{
	D3DXMatrixRotationYawPitchRoll
	(
		&rootAxis
		, Math::ToRadian(rotation.y)
		, Math::ToRadian(rotation.x)
		, Math::ToRadian(rotation.z)
	);
}

void GameRender::RootAxis(float x, float y, float z)
{
	D3DXVECTOR3 rotation = D3DXVECTOR3(x, y, z);

	RootAxis(rotation);
}

D3DXMATRIX GameRender::RootAxis()
{
	return rootAxis;
}

D3DXMATRIX GameRender::World()
{
	return world;
}

void GameRender::Position(D3DXVECTOR3 & vec)
{
	position = vec;

	UpdateWorld();
}

void GameRender::Position(float x, float y, float z)
{
	D3DXVECTOR3 val(x, y, z);

	Position(val);
}

D3DXVECTOR3 GameRender::Position()
{
	return position;
}

void GameRender::Scale(D3DXVECTOR3 & vec)
{
	scale = vec;

	UpdateWorld();
}

void GameRender::Scale(float x, float y, float z)
{
	D3DXVECTOR3 val(x, y, z);

	Scale(val);
}

D3DXVECTOR3 GameRender::Scale()
{
	return scale;
}

void GameRender::Rotation(D3DXVECTOR3 & vec)
{
	rotation = vec;

	UpdateWorld();
}

void GameRender::Rotation(float x, float y, float z)
{
	D3DXVECTOR3 val(x, y, z);

	Rotation(val);
}

void GameRender::RotationDegree(D3DXVECTOR3 & vec)
{
	D3DXVECTOR3 temp;
	temp.x = Math::ToRadian(vec.x);
	temp.y = Math::ToRadian(vec.y);
	temp.z = Math::ToRadian(vec.z);

	Rotation(temp);
}

void GameRender::RotationDegree(float x, float y, float z)
{
	D3DXVECTOR3 val(x, y, z);

	RotationDegree(val);
}

D3DXVECTOR3 GameRender::Rotation()
{
	return rotation;
}

D3DXVECTOR3 GameRender::RotationDegree()
{
	D3DXVECTOR3 temp;
	temp.x = Math::ToDegree(rotation.x);
	temp.y = Math::ToDegree(rotation.y);
	temp.z = Math::ToDegree(rotation.z);

	return temp;
}

D3DXVECTOR3 GameRender::Forward()
{
	return forward;
}

D3DXVECTOR3 GameRender::Up()
{
	return up;
}

D3DXVECTOR3 GameRender::Right()
{
	return right;
}

D3DXMATRIX GameRender::Transformed()
{
	return rootAxis * world;
}

void GameRender::Update()
{
}

void GameRender::Render()
{
}


void GameRender::UpdateWorld()
{
	D3DXMATRIX S, R, T;

	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXMatrixRotationYawPitchRoll(&R, rotation.y, rotation.x, rotation.z);
	D3DXMatrixTranslation(&T, position.x, position.y, position.z);

	world = S * R * T;

	forward = D3DXVECTOR3(world._31, world._32, world._33);
	up = D3DXVECTOR3(world._21, world._22, world._23);
	right = D3DXVECTOR3(world._11, world._12, world._13);
}
