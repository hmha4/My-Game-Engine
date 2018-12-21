#include "stdafx.h"
#include "Program.h"
#include "./Viewer/Freedom.h"
#include "./Viewer/Perspective.h"
#include "./Viewer/Viewport.h"

#include "./Executes/ExportMesh.h"
#include "./Executes/TessTerrain.h"

#include "./Model/Model.h"

Program::Program()
{
	Context::Create();
	
	Context::Get()->GetMainCamera()->RotationDegree(0, 0);
	Context::Get()->GetMainCamera()->Position(0, 0, -5);

	((Freedom *)Context::Get()->GetMainCamera())->MoveSpeed(10.0f);


	//executes.push_back(new ExportMesh());
	executes.push_back(new TessTerrain());
}

Program::~Program()
{
	for (Execute* exe : executes)
		SAFE_DELETE(exe);

	Context::Delete();
	Models::Delete();
	Textures::Delete();
	Shaders::Delete();
}

void Program::Update()
{
	Context::Get()->Update();

	for (Execute* exe : executes)
		exe->Update();
}

void Program::PreRender()
{
	for (Execute* exe : executes)
		exe->PreRender();
	
	//Context::Get()->Update();
}

void Program::Render()
{
	//	PreRender에서 RTV 설정할 때 변경 해줬으므로 다시 원상 복귀시켜줌
	//Context::Get()->GetViewport()->RSSetViewport();
	
	for (Execute* exe : executes)
		exe->Render();
}

void Program::PostRender()
{
	for (Execute* exe : executes)
		exe->PostRender();

	wstring str = String::Format(L"FPS : %.0f", ImGui::GetIO().Framerate);
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

void Program::ResizeScreen()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	Context::Get()->GetPerspective()->Set(desc.Width, desc.Height);
	Context::Get()->GetViewport()->Set(desc.Width, desc.Height);

	for (Execute* exe : executes)
		exe->ResizeScreen();
}