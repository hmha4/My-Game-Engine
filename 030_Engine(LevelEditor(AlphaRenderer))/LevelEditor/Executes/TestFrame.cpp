#include "stdafx.h"
#include "TestFrame.h"
#include "Editors/EditorManager.h"
#include "Scene/GameScene.h"
#include "Environments/Shadow.h"

void TestFrame::Initialize()
{
	
}

void TestFrame::Ready()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	shadow = new Shadow(desc.Width, desc.Height);
	shadow->Initialize();
	shadow->Ready();

	//effect = new Effect(Effects + L"033_AnimInstance.fx");
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			GameAnimator * model = new GameAnimator
			(
				i * 5 + j, NULL,
				Models + L"skeletonzombie_t_avelange/skeletonzombie_t_avelange.material",
				Models + L"skeletonzombie_t_avelange/skeletonzombie_t_avelange.mesh"
			);
			model->Ready();
			model->Position(i * 2, 0, j * 2);

			model->AddClip(1, Models + L"skeletonzombie_t_avelange/Idle.anim");
			model->Play("Idle", true);

			models.push_back(model);
			shadow->Add(model);
		}
	}

	meshMat = new Material(Effects + L"013_Mesh&Model.fx");
	plane = new MeshGrid(meshMat, 10, 10, 20, 20);
	plane->GetMaterial()->SetDiffuseMap(Textures + L"White.png");
	plane->Position(5, 0, 5);
	shadow->Add(plane);
}

void TestFrame::Destroy()
{
	
}

void TestFrame::Update()
{
	shadow->Update();
	//for (GameAnimator * model : models)
	//	model->Update();
}

void TestFrame::PreRender()
{
	shadow->PreRender();
}

void TestFrame::Render()
{
	shadow->Render();
	//InstanceManager::Get()->Render(0);

	ImGui::Begin("Test");
	{
		int fps = Time::Get()->FPS();
		ImGui::Text("FPS : %d", fps);

		ImGui::DragFloat3("Direction", (float*)&Context::Get()->GetGlobalLight()->Direction, 0.01f, -1, 1);
		Context::Get()->ChangeGlobalLight();
	}
	ImGui::End();
}

void TestFrame::PostRender()
{
}

void TestFrame::ResizeScreen()
{
}