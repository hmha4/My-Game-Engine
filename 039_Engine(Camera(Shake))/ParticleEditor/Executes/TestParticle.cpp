#include "stdafx.h"
#include "TestParticle.h"
#include "ParticleTool/ParticleTool.h"
#include "Particles/Fire.h"

#include "Module/LightningManager.h"

void TestParticle::Initialize()
{
	tool = NULL;
	tool = new ParticleTool();
	tool->Initialize();
}

void TestParticle::Ready()
{
	tool->Ready();

}

void TestParticle::Destroy()
{
	SAFE_DELETE(tool);
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
}

void TestParticle::PostRender()
{
	tool->PostRender();
}

void TestParticle::ResizeScreen()
{
}