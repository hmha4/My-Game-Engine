#include "stdafx.h"
#include "TestModel.h"
#include "Fbx/FbxLoader.h"

void TestModel::Initialize()
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

void TestModel::Ready()
{
	/*model = new GameModel
	(
		Effects + L"003_Model&Animation.fx",
		Models + L"Kachujin/Kachujin.material",
		Models + L"Kachujin/Kachujin.mesh"
	);*/

	model = new GameAnimator(
		Effects + L"003_Model&Animation.fx",
		Models + L"Kachujin/Kachujin.material",
		Models + L"Kachujin/Kachujin.mesh"
	);

	model->AddClip(Models + L"Kachujin/Samba_Dancing.anim");
}

void TestModel::Destroy()
{
	SAFE_DELETE(model);
}

void TestModel::Update()
{
	model->Update();
}

void TestModel::PreRender()
{
	
}

void TestModel::Render()
{
	model->Render();

	ImGui::DragFloat3("Direction", (float*)Context::Get()->GetGlobalLight()->Direction, 0.01f, -1, 1);
	Context::Get()->ChangeGlobalLight();
}

void TestModel::PostRender()
{
}

void TestModel::ResizeScreen()
{
}
