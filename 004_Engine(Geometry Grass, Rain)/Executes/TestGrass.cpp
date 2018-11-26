#include "stdafx.h"
#include "TestGrass.h"
#include "../Environment/Terrain.h"
#include "../Environment/Grass.h"
#include "../Environment/Rain.h"

TestGrass::TestGrass()
{
	terrain = new Terrain();
	grass = new Grass(terrain->Width(), terrain);
	rain = new Rain(D3DXVECTOR3(100, 100, 100), 5000);
}

TestGrass::~TestGrass()
{
	SAFE_DELETE(terrain);
	SAFE_DELETE(grass);
	SAFE_DELETE(rain);
}

void TestGrass::Update()
{
	grass->Update();
	rain->Update();
}

void TestGrass::PreRender()
{
}

void TestGrass::Render()
{
	terrain->Render();
	grass->Render();
	rain->Render();
}

void TestGrass::PostRender()
{

}

void TestGrass::ResizeScreen()
{
}