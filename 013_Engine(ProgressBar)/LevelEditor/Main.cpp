#include "stdafx.h"
#include "Main.h"
#include "Systems/Window.h"
#include "Viewer/Freedom.h"

#include "Executes/TestTerrain.h"

void Main::Initialize()
{
	Context::Create();

	Context::Get()->GetMainCamera()->RotationDegree(0, 0);
	Context::Get()->GetMainCamera()->Position(0, 0, -5);

	((Freedom *)Context::Get()->GetMainCamera())->MoveSpeed(10.0f);

	Push(new TestTerrain());
}

void Main::Ready()
{
	for (IExecute* exe : executes)
		exe->Ready();
}

void Main::Destroy()
{
	for (IExecute* exe : executes)
	{
		exe->Destroy();
		SAFE_DELETE(exe);
	}

	Context::Delete();
	Models::Delete();
	Textures::Delete();
	Shaders::Delete();
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
	//Context::Get()->GetViewport()->RSSetViewport();

	for (IExecute* exe : executes)
		exe->Render();
}

void Main::PostRender()
{
	for (IExecute* exe : executes)
		exe->PostRender();

	wstring str = String::Format(L"FPS : %.0f", Time::Get()->FPS());
	RECT rect = { 0, 0, 300, 300 };
	DirectWrite::RenderText(str, rect, 12);

	rect.top += 12;
	D3DXVECTOR3 vec;
	Context::Get()->GetMainCamera()->Position(&vec);
	str = String::Format(L"CameraPos : %.0f, %.0f, %.0f", vec.x, vec.y, vec.x);
	DirectWrite::RenderText(str, rect, 12);

	rect.top += 12;
	D3DXVECTOR2 rot;
	Context::Get()->GetMainCamera()->Rotation(&rot);
	str = String::Format(L"CameraRot : %.0f, %.0f", rot.x, rot.y);
	DirectWrite::RenderText(str, rect, 12);
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
	desc.Width = 1280;
	desc.Height = 720;
	D3D::SetDesc(desc);

	Main* main = new Main();
	WPARAM wParam = Window::Run(main);

	SAFE_DELETE(main);

	return wParam;
}
