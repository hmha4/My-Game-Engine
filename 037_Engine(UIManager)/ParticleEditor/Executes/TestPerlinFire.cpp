#include "stdafx.h"
#include "TestPerlinFire.h"
#include "Particles/Fire.h"

void TestPerlinFire::Initialize()
{
	fire = new Fire();
	fire->Initialize();
	AlphaRenderer::Get()->AddAlphaMaps(fire);
}

void TestPerlinFire::Ready()
{
	fire->Ready(0, 0.5f, 0);
	fire->Setting(1, 4, 0.6f, 1.35f, 3, 1);
}

void TestPerlinFire::Destroy()
{
	SAFE_DELETE(fire);
}

void TestPerlinFire::Update()
{

}

void TestPerlinFire::PreRender()
{
}

void TestPerlinFire::Render()
{
	//fire->Render();
	fire->ImGuiRender();
}

void TestPerlinFire::PostRender()
{

}

void TestPerlinFire::ResizeScreen()
{
}