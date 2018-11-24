#include "stdafx.h"
#include "TestLine.h"

TestLine::TestLine()
{
	bBox = new BBox();
	bBox->Min = D3DXVECTOR3(0, 0, 0);
	bBox->Max = D3DXVECTOR3(1, 1, 1);
	boxLine = new DebugLine();

	D3DXMATRIX W; D3DXMatrixIdentity(&W);
	boxLine->Color(1, 0, 0);
	boxLine->Draw(W, bBox);

	cubeMat = new Material(Effects + L"002_Mesh.fx");
	cubeMat->SetDiffuseMap(Textures + L"Floor.png");

	cube = new MeshCube(cubeMat, 1, 1, 1);
	cube->Position(0.5f, 0.5f, 0.5f);
}

TestLine::~TestLine()
{
	SAFE_DELETE(cube);
	SAFE_DELETE(cubeMat);

	SAFE_DELETE(boxLine);
	SAFE_DELETE(bBox);
}

void TestLine::Update()
{
	
}

void TestLine::PreRender()
{
}

void TestLine::Render()
{
	boxLine->Render();
	cube->Render();
}

void TestLine::PostRender()
{

}

void TestLine::ResizeScreen()
{
}