#include "Framework.h"
#include "Frustum.h"
#include "Camera.h"
#include "Perspective.h"

Frustum::Frustum(float zFar, Camera * camera, Perspective * perspective)
	: zFar(zFar), camera(camera), perspective(perspective)
{
	if (camera == NULL)
		this->camera = Context::Get()->GetMainCamera();

	if (perspective == NULL)
		this->perspective = Context::Get()->GetPerspective();
}

Frustum::~Frustum()
{
}

void Frustum::Update()
{
	D3DXMATRIX V, P;
	camera->MatrixView(&V);
	perspective->GetMatrix(&P);

	float zNear = -P._43 / P._33;
	float r = zFar / (zFar - zNear);

	P._33 = r;
	P._43 = -r * zNear;

	D3DXMATRIX M;
	D3DXMatrixMultiply(&M, &V, &P);

	//Near
	planes[0].a = M._14 + M._13;
	planes[0].b = M._24 + M._23;
	planes[0].c = M._34 + M._33;
	planes[0].d = M._44 + M._43;

	//Far
	planes[1].a = M._14 - M._13;
	planes[1].b = M._24 - M._23;
	planes[1].c = M._34 - M._33;
	planes[1].d = M._44 - M._43;

	//Left
	planes[2].a = M._14 + M._11;
	planes[2].b = M._24 + M._21;
	planes[2].c = M._34 + M._31;
	planes[2].d = M._44 + M._41;

	//Right
	planes[3].a = M._14 - M._11;
	planes[3].b = M._24 - M._21;
	planes[3].c = M._34 - M._31;
	planes[3].d = M._44 - M._41;

	//Top
	planes[4].a = M._14 - M._12;
	planes[4].b = M._24 - M._22;
	planes[4].c = M._34 - M._32;
	planes[4].d = M._44 - M._42;

	//Bottom
	planes[5].a = M._14 + M._12;
	planes[5].b = M._24 + M._22;
	planes[5].c = M._34 + M._32;
	planes[5].d = M._44 + M._42;

	for (int i = 0; i < 6; i++)
		D3DXPlaneNormalize(&planes[i], &planes[i]);
}

void Frustum::GetPlanes(OUT D3DXPLANE * plane)
{
	D3DXMATRIX V, P;
	camera->MatrixView(&V);
	perspective->GetMatrix(&P);

	float zNear = -P._43 / P._33;
	float r = zFar / (zFar - zNear);

	P._33 = r;
	P._43 = -r * zNear;

	D3DXMATRIX M;
	D3DXMatrixMultiply(&M, &V, &P);

	//Left
	plane[0].a = M._14 + M._11;
	plane[0].b = M._24 + M._21;
	plane[0].c = M._34 + M._31;
	plane[0].d = M._44 + M._41;

	//Right
	plane[1].a = M._14 - M._11;
	plane[1].b = M._24 - M._21;
	plane[1].c = M._34 - M._31;
	plane[1].d = M._44 - M._41;

	//Bottom
	plane[2].a = M._14 + M._12;
	plane[2].b = M._24 + M._22;
	plane[2].c = M._34 + M._32;
	plane[2].d = M._44 + M._42;

	//Top
	plane[3].a = M._14 - M._12;
	plane[3].b = M._24 - M._22;
	plane[3].c = M._34 - M._32;
	plane[3].d = M._44 - M._42;

	//Near
	plane[4].a = M._14 + M._13;
	plane[4].b = M._24 + M._23;
	plane[4].c = M._34 + M._33;
	plane[4].d = M._44 + M._43;

	//Far
	plane[5].a = M._14 - M._13;
	plane[5].b = M._24 - M._23;
	plane[5].c = M._34 - M._33;
	plane[5].d = M._44 - M._43;

	for (int i = 0; i < 6; i++)
		D3DXPlaneNormalize(&plane[i], &plane[i]);
}

bool Frustum::ContainPoint(D3DXVECTOR3 & position)
{
	for (int i = 0; i < 6; i++)
	{
		if (D3DXPlaneDotCoord(&planes[i], &position) < 0.0f)
			return false;
	}

	return true;
}

bool Frustum::ContainRect(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize)
{
	for (int i = 0; i < 6; i++)
	{
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter - ySize), (zCenter - zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter - ySize), (zCenter - zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter + ySize), (zCenter - zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter - ySize), (zCenter + zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter + ySize), (zCenter - zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter - ySize), (zCenter + zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter + ySize), (zCenter + zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter + ySize), (zCenter + zSize))) >= 0.0f)
			continue;

		return false;
	}

	return true;
}

bool Frustum::ContainRect(D3DXVECTOR3 center, D3DXVECTOR3 size)
{
	return ContainRect(center.x, center.y, center.z, size.x, size.y, size.z);
}

bool Frustum::ContainCube(D3DXVECTOR3 & center, float radius)
{
	D3DXVECTOR3 check;

	for (int i = 0; i < 6; i++)
	{
		check.x = center.x - radius;
		check.y = center.y - radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y - radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y + radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y + radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y - radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y - radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y + radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y + radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		return false;
	}

	return true;
}