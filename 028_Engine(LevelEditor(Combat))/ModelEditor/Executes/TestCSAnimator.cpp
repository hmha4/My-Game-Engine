#include "stdafx.h"
#include "TestCSAnimator.h"
#include "Fbx/FbxLoader.h"
#include "Module/TrailRenderer.h"


void TestCSAnimator::Initialize()
{
	/*FbxLoader * loader = NULL;

	loader = new FbxLoader
	(
		Assets + L"Brute/Brute.fbx",
		Models + L"Brute/", L"Brute"
	);
	loader->ExportMaterial();
	loader->ExportMesh();
	SAFE_DELETE(loader);

	loader = new FbxLoader
	(
		Assets + L"Brute/Samba_Dancing.fbx",
		Models + L"Brute/", L"Samba_Dancing"
	);
	loader->ExportAnimation(0);
	SAFE_DELETE(loader);*/
}

void TestCSAnimator::Ready()
{
	model = new GameAnimator
	(
		Effects + L"026_Model.fx",
		Models + L"Brute/Brute.material",
		Models + L"Brute/Brute.mesh"
	);
	model->Ready();
	model->AddClip(Models + L"Brute/Idle.anim");
	model->AddClip(Models + L"Brute/Attack.anim");
	//model->Play("Idle", true);

	trailEffect = new Effect(Effects + L"031_TrailRender.fx");
	trail = new TrailRenderer(model->GetModel(), trailEffect);
	trail->Ready();
	//trail->SetBone(L"RightHand");
}

void TestCSAnimator::Destroy()
{
	SAFE_DELETE(model);
	SAFE_DELETE(trail);
}

void TestCSAnimator::Update()
{
	model->Update();
	//trail->Update(0.2f);

	if (Keyboard::Get()->Down('K'))
	{
		trail->SetRun(true);
		trail->ResetTrail();
		model->Play("Attack", false);
	}
		

	if (Keyboard::Get()->Down('J'))
	{
		trail->SetRun(false);
		trail->ResetTrail();
		model->Play("Idle", true);
	}

	if (Keyboard::Get()->Down('P'))
	{
		trail->SetRun(false);
		model->Pause();
	}
}

void TestCSAnimator::PreRender()
{
}

void TestCSAnimator::Render()
{
	model->Render();
	trail->Render();
	trail->ImGuiRender();
	/*ImGui::ColorEdit3("Ambient", (float*)&Context::Get()->GetGlobalLight()->Ambient);
	ImGui::ColorEdit3("Diffuse", (float*)&Context::Get()->GetGlobalLight()->Diffuse);
	ImGui::DragFloat4("Specular", (float*)&Context::Get()->GetGlobalLight()->Specular, 0.01f, 0.1f, 30.0f);
	ImGui::DragFloat3("Direction", (float*)&Context::Get()->GetGlobalLight()->Direction, 0.01f, -1, 1);
	Context::Get()->ChangeGlobalLight();*/
}

void TestCSAnimator::PostRender()
{
	trail->PostRender();
}

void TestCSAnimator::ResizeScreen()
{
}