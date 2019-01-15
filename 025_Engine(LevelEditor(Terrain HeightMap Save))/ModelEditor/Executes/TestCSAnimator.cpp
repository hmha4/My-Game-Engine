#include "stdafx.h"
#include "TestCSAnimator.h"
#include "Fbx/FbxLoader.h"

void TestCSAnimator::Initialize()
{
	/*FbxLoader * loader = NULL;

	loader = new FbxLoader
	(
		Assets + L"Kachujin/Kachujin.fbx",
		Models + L"Kachujin/", L"Kachujin"
	);
	loader->ExportMaterial();
	loader->ExportMesh();
	SAFE_DELETE(loader);

	loader = new FbxLoader
	(
		Assets + L"Kachujin/Samba_Dancing.fbx",
		Models + L"Kachujin/", L"Samba_Dancing"
	);
	loader->ExportAnimation(0);
	SAFE_DELETE(loader);*/
}

void TestCSAnimator::Ready()
{
	model = new GameAnimator
	(
		Effects + L"026_Model.fx",
		Models + L"Kachujin/Kachujin.material",
		Models + L"Kachujin/Kachujin.mesh"
	);

	model->AddClip(Models + L"Kachujin/Samba_Dancing.anim");
}

void TestCSAnimator::Destroy()
{
	SAFE_DELETE(model);
}

void TestCSAnimator::Update()
{
	model->Update();
}

void TestCSAnimator::PreRender()
{
}

void TestCSAnimator::Render()
{
	model->Render();

	ImGui::ColorEdit3("Ambient", (float*)&Context::Get()->GetGlobalLight()->Ambient);
	ImGui::ColorEdit3("Diffuse", (float*)&Context::Get()->GetGlobalLight()->Diffuse);
	ImGui::DragFloat4("Specular", (float*)&Context::Get()->GetGlobalLight()->Specular, 0.01f, 0.1f, 30.0f);
	ImGui::DragFloat3("Direction", (float*)&Context::Get()->GetGlobalLight()->Direction, 0.01f, -1, 1);
	Context::Get()->ChangeGlobalLight();
}

void TestCSAnimator::PostRender()
{
}

void TestCSAnimator::ResizeScreen()
{
}