#include "stdafx.h"
#include "ParticleTool.h"
#include "Particles\ParticleSystem.h"
#include "Draw/GizmoGrid.h"

ParticleTool::ParticleTool()
{
}

ParticleTool::~ParticleTool()
{
	SAFE_DELETE(pSystem);
	SAFE_DELETE(grid);
}

void ParticleTool::Initialize()
{

}

void ParticleTool::Ready()
{
	vector<wstring> names;
	names.push_back(Textures + L"blue-effect.png");

	pSystem = new ParticleSystem(names, 1);
	pSystem->Initialize(15.0f, 0.2f, D3DXVECTOR2(1, 1), D3DXVECTOR2(1, 1), D3DXVECTOR2(1, 1), 30.0f, 1.0f, 0.0f);
	pSystem->Ready();


	grid = new GizmoGrid(1);
	grid->Initialize();
	grid->Ready();
}

void ParticleTool::Update()
{
	pSystem->EmitParticles();
	pSystem->KillParticle();

	if (Keyboard::Get()->Down('1'))
		pSystem->ResetParticle();
}

void ParticleTool::PreRender()
{
}

void ParticleTool::Render()
{
	if (DEFERRED == false)
	{
		grid->Render();
		pSystem->Render();
	}
		

	//MyGui::ShowDebugOverlay();

	//if (ImGui::BeginMainMenuBar())
	//{
	//	if (ImGui::BeginMenu("File"))
	//	{
	//		if (ImGui::MenuItem("Open"))
	//		{
	//		}
	//		if (ImGui::MenuItem("Save"))
	//		{
	//		}
	//		if (ImGui::MenuItem("Exit"))
	//		{
	//			PostQuitMessage(0);
	//		}
	//		ImGui::EndMenu();
	//	}
	//	if (ImGui::BeginMenu("Edit"))
	//	{
	//		ImGui::MenuItem("Style", NULL, &MyGui::show_app_style);
	//		ImGui::EndMenu();
	//	}
	//	if (ImGui::BeginMenu("Windows"))
	//	{
	//		ImGui::MenuItem("Light", NULL, &MyGui::show_app_light);
	//		ImGui::MenuItem("Camera", NULL, &MyGui::show_app_camera);
	//		ImGui::MenuItem("Inspector", NULL, &MyGui::show_app_inspector);
	//		ImGui::MenuItem("Hierarchy", NULL, &MyGui::show_app_hierarchy);
	//		ImGui::MenuItem("Animation", NULL, &MyGui::show_app_animation);
	//		ImGui::MenuItem("Behaviour", NULL, &MyGui::show_app_behaviour);
	//		ImGui::MenuItem("Debug Log", NULL, &MyGui::show_app_debugLog);
	//		ImGui::EndMenu();
	//	}
	//	if (ImGui::BeginMenu("Help"))
	//	{
	//		ImGui::MenuItem("help", NULL, &MyGui::show_app_help);
	//		ImGui::EndMenu();
	//	}
	//	ImGui::EndMainMenuBar();
	//}
	////ImGui::ShowDemoWindow();

	//if (MyGui::show_app_help) { MyGui::ShowUserGuide(); }
	//if (MyGui::show_app_style) { MyGui::ShowStyleEdit(); }
	//if (MyGui::show_app_camera) { MyGui::ShowCameraSetting(Context::Get()); }
	//if (MyGui::show_app_light) { MyGui::ShowLightSetting(Context::Get()); }
	//if (MyGui::show_app_debugLog) { MyGui::DrawLog(); }

	//if (MyGui::show_app_hierarchy) { RenderHeirarchy(); }
	//if (MyGui::show_app_inspector) { RenderInspector(); }
	pSystem->ImGuiRender();
}

void ParticleTool::PostRender()
{
}

void ParticleTool::Delete()
{
}

void ParticleTool::Reset()
{
}

void ParticleTool::RenderHeirarchy()
{
	ImGui::Begin("Hierarchy", &MyGui::show_app_hierarchy, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove);
	{

	}
	ImGui::End();
}

void ParticleTool::RenderInspector()
{
	ImGui::Begin("Inspector", &MyGui::show_app_inspector, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoMove);
	{

	}
	ImGui::End();
}

void ParticleTool::Save()
{

}

void ParticleTool::Load(wstring fileName)
{

}
