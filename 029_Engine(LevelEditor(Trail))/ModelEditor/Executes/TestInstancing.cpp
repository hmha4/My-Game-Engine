#include "stdafx.h"
#include "TestInstancing.h"
#include "Fbx/FbxLoader.h"
#include "Module/TrailRenderer.h"


void TestInstancing::Initialize()
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

void TestInstancing::Ready()
{
	effect = new Effect(Effects + L"033_AnimInstance.fx");
	for (int i = 0; i < 10; i++)
	{
		//model = new GameModel
		//(
		//	i,
		//	Models + L"Tank/Tank.material",
		//	Models + L"Tank/Tank.mesh"
		//);
		//model->Ready();
		//model->Position(Math::Random(-50, 50), 0, Math::Random(-50, 50));

		model = new GameAnimator
		(
			i, effect,
			Models + L"Brute/Brute.material",
			Models + L"Brute/Brute.mesh"
		);
		model->Ready();
		//model->Scale(0.001f, 0.001f, 0.001f);
		model->Position(Math::Random(-20.0f, 20.0f), 0, Math::Random(-20.0f, 20.0f));

		model->AddClip(1, Models + L"Brute/Idle.anim");
		model->AddClip(2, Models + L"Brute/Attack.anim");
		model->Play("Attack", true);

		//models.emplace_back(model);
	}
	

	

	//trailEffect = new Effect(Effects + L"031_TrailRender.fx");
	//trail = new TrailRenderer(model->GetModel(), trailEffect);
	//trail->Ready();
	//trail->SetBone(L"RightHand");
}

void TestInstancing::Destroy()
{
	//for (GameAnimator * model : models)
		SAFE_DELETE(model);
	//SAFE_DELETE(trail);
	SAFE_DELETE(effect);
}

void TestInstancing::Update()
{
	//for (GameAnimator * model : models)
		model->Update();
	//trail->Update(0.2f);
}

void TestInstancing::PreRender()
{
}

void TestInstancing::Render()
{
	InstanceManager::Get()->Render();
	//model->Render();
	//trail->Render();
	//trail->ImGuiRender();
	/*ImGui::ColorEdit3("Ambient", (float*)&Context::Get()->GetGlobalLight()->Ambient);
	ImGui::ColorEdit3("Diffuse", (float*)&Context::Get()->GetGlobalLight()->Diffuse);
	ImGui::DragFloat4("Specular", (float*)&Context::Get()->GetGlobalLight()->Specular, 0.01f, 0.1f, 30.0f);
	ImGui::DragFloat3("Direction", (float*)&Context::Get()->GetGlobalLight()->Direction, 0.01f, -1, 1);
	Context::Get()->ChangeGlobalLight();*/
}

void TestInstancing::PostRender()
{
	//trail->PostRender();
}

void TestInstancing::ResizeScreen()
{
}