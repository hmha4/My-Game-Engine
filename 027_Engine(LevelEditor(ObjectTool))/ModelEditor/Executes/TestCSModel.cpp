#include "stdafx.h"
#include "TestCSModel.h"
#include "Fbx/FbxLoader.h"

void TestCSModel::Initialize()
{
	FbxLoader * loader = NULL;

	loader = new FbxLoader
	(
		Assets + L"Tank/Tank.fbx",
		Models + L"Tank/", L"Tank"
	);
	loader->ExportMaterial();
	loader->ExportMesh();
	SAFE_DELETE(loader);
}

void TestCSModel::Ready()
{
	model = new GameModel
	(
		Effects + L"026_Model.fx",
		Models + L"Tank/Tank.material",
		Models + L"Tank/Tank.mesh"
	);
	model->Ready();
}

void TestCSModel::Destroy()
{
	SAFE_DELETE(model);
}

void TestCSModel::Update()
{
	model->Update();
}

void TestCSModel::PreRender()
{
}

void TestCSModel::Render()
{
	model->Render();

	/*ImGui::ColorEdit3("Ambient", (float*)&Context::Get()->GetGlobalLight()->Ambient);
	ImGui::ColorEdit3("Diffuse", (float*)&Context::Get()->GetGlobalLight()->Diffuse);
	ImGui::DragFloat4("Specular", (float*)&Context::Get()->GetGlobalLight()->Specular, 0.01f, 0.1f, 30.0f);
	ImGui::DragFloat3("Direction", (float*)&Context::Get()->GetGlobalLight()->Direction, 0.01f, -1, 1);
	Context::Get()->ChangeGlobalLight();*/
}

void TestCSModel::PostRender()
{
}

void TestCSModel::ResizeScreen()
{
}