#include "stdafx.h"
#include "Main.h"
#include "Systems/Window.h"
#include "Viewer/Freedom.h"

#include "Executes/TestMapEditor.h"


void Main::Initialize()
{
	weaponColLayer = CollisionContext::Get()->AddLayer(L"Collision Weapon");
	eWeaponColLayer = CollisionContext::Get()->AddLayer(L"Collision Enemy Weapon");
	itemColLayer = CollisionContext::Get()->AddLayer(L"Collision Item");
	playerColLayer = CollisionContext::Get()->AddLayer(L"Collision Player");
	enemyColLayer = CollisionContext::Get()->AddLayer(L"Collision Enemy");
	characterColLayer = CollisionContext::Get()->AddLayer(L"Collision Character");
	worldColLayer = CollisionContext::Get()->AddLayer(L"Collision World");

	Context::Get()->GetMainCamera()->RotationDegree(0, 0);
	Context::Get()->GetMainCamera()->Position(0, 0, -10);

	((Freedom *)Context::Get()->GetMainCamera())->MoveSpeed(50.0f);

	Push(new TestMapEditor());
}

void Main::Ready()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	for (IExecute* exe : executes)
		exe->Ready();
}

void Main::Destroy()
{
	InstanceManager::Delete();

	for (IExecute* exe : executes)
	{
		exe->Destroy();
		SAFE_DELETE(exe);
	}

	LightningManager::Delete();
	ParticleManager::Delete();
	DeferredRendering::Delete();
	Context::Delete();
	Models::Delete();
	Textures::Delete();
	Effects::Delete();
}

void Main::Update()
{
	Context::Get()->Update();

	for (IExecute* exe : executes)
		exe->Update();
}

void Main::PreRender()
{
	for (IExecute* exe : executes)
		exe->PreRender();
}

void Main::Render()
{
	//	PreRender에서 RTV 설정할 때 변경 해줬으므로 다시 원상 복귀시켜줌
	Context::Get()->GetViewport()->RSSetViewport();

	for (IExecute* exe : executes)
		exe->Render();
}

void Main::PostRender()
{
	for (IExecute* exe : executes)
		exe->PostRender();

	//CollisionContext::Get()->Render();

	//ImGui::SetNextWindowPos(ImVec2(-100, -100));
	//ImGui::Begin("BCKGND", NULL, ImGui::GetIO().DisplaySize, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
	//{
	//	int fps = Time::Get()->FPS();
	//	string str = "FPS : " + to_string(fps);
	//	ImGui::GetWindowDrawList()->AddText(ImVec2(0, 0), ImColor(1.0f, 1.0f, 1.0f, 1.0f), str.c_str());
	//
	//	D3DXVECTOR3 vec;
	//	Context::Get()->GetMainCamera()->Position(&vec);
	//	str = "CameraPos : " + to_string((int)vec.x) + ", " + to_string((int)vec.y) + ", " + to_string((int)vec.z);
	//	ImGui::GetWindowDrawList()->AddText(ImVec2(0, 12), ImColor(1.0f, 1.0f, 1.0f, 1.0f), str.c_str());
	//
	//	D3DXVECTOR2 rot;
	//	Context::Get()->GetMainCamera()->Rotation(&rot);
	//	str = "CameraRot : " + to_string((int)rot.x) + ", " + to_string((int)rot.y);
	//	ImGui::GetWindowDrawList()->AddText(ImVec2(0, 24), ImColor(1.0f, 1.0f, 1.0f, 1.0f), str.c_str());
	//}
	//ImGui::End();
}

void Main::ResizeScreen()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	if (Context::Get() != NULL)
	{
		Context::Get()->GetPerspective()->Set(desc.Width, desc.Height);
		Context::Get()->GetViewport()->Set(desc.Width, desc.Height);
	}

	for (IExecute* exe : executes)
		exe->ResizeScreen();
}

void Main::Push(IExecute * execute)
{
	executes.push_back(execute);

	execute->Initialize();
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR param, int command)
{
	D3DDesc desc;
	desc.AppName = L"D3D Game";
	desc.Instance = instance;
	desc.bFullScreen = false;
	desc.bVsync = false;
	desc.Handle = NULL;
	desc.Width = 1600;
	desc.Height = 900;
	D3D::SetDesc(desc);

	Main* main = new Main();
	WPARAM wParam = Window::Run(main);

	SAFE_DELETE(main);

	return wParam;
}