#include "stdafx.h"
#include "TestParticle.h"
#include "ParticleTool/ParticleTool.h"
#include "Particles/Fire.h"

void TestParticle::Initialize()
{
	tool = NULL;
	tool = new ParticleTool();
	tool->Initialize();

	//fire = new Fire();
	//fire->Initialize();
	//
	//AlphaRenderer::Get()->AddAlphaMaps(fire);
}

void TestParticle::Ready()
{
	tool->Ready();
	//fire->Ready(0, 0.5f, 0);
	//fire->Setting(1, 4, 0.6, 1.35, 3, 1);
}

void TestParticle::Destroy()
{
	SAFE_DELETE(tool);
	//SAFE_DELETE(fire);
}

void TestParticle::Update()
{
	tool->Update();
}

void TestParticle::PreRender()
{
	tool->PreRender();
}

void TestParticle::Render()
{
	tool->Render();
	//fire->ImGuiRender();
}

void TestParticle::PostRender()
{
	tool->PostRender();
}

void TestParticle::ResizeScreen()
{
}