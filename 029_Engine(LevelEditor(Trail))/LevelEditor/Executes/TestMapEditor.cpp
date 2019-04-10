#include "stdafx.h"
#include "TestMapEditor.h"
#include "Editors/EditorManager.h"
#include "Scene/GameScene.h"
#include "Environments/Shadow.h"

void TestMapEditor::Initialize()
{
	editorMgr = new EditorManager();
	editorMgr->Initialize();

	gameScene = new GameScene();
	gameScene->Initialize();
	gameScene->InputShadow(editorMgr->GetShadow());

	playButton = NULL;
	stopButton = NULL;

	corner = 1;
	distance = 10.0f;

	mode = TestMapEditor::Mode::EDIT;
}

void TestMapEditor::Ready()
{
	playButton = new Texture(Textures + L"PlayButton.png");
	stopButton = new Texture(Textures + L"StopButton.png");

	editorMgr->Ready();
	gameScene->Ready();
}

void TestMapEditor::Destroy()
{
	SAFE_DELETE(gameScene);
	SAFE_DELETE(editorMgr);

	SAFE_DELETE(playButton);
	SAFE_DELETE(stopButton);
}

void TestMapEditor::Update()
{
	switch (mode)
	{
	case TestMapEditor::Mode::EDIT:
		editorMgr->Update();

		break;
	case TestMapEditor::Mode::GAME:
		gameScene->Update();
		break;
	}
}

void TestMapEditor::PreRender()
{
	switch (mode)
	{
	case TestMapEditor::Mode::EDIT:
		editorMgr->PreRender();

		break;
	case TestMapEditor::Mode::GAME:
		gameScene->PreRender();
		break;
	}
}

void TestMapEditor::Render()
{
	MyGui::ShowDebugOverlay();

	ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x / 2 - distance : distance, (corner & 2) ? ImGui::GetIO().DisplaySize.y - distance * 3 : distance * 3);
	ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
	if (corner != -1)
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
	ImGui::Begin("", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
	{
		if (ImGui::ImageButton(playButton->SRV(), ImVec2(20, 20)))
		{
			if (mode == Mode::EDIT)
			{
				gameScene->InitObjects(editorMgr->GameObjects());
			}
			mode = Mode::GAME;
		}
		ImGui::SameLine();
		if (ImGui::ImageButton(stopButton->SRV(), ImVec2(20, 20)))
		{
			if (mode == Mode::GAME)
				gameScene->ClearObjects();
			mode = Mode::EDIT;
		}

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			//if (p_open && ImGui::MenuItem("Close")) *p_open = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();

	switch (mode)
	{
	case TestMapEditor::Mode::EDIT:
		editorMgr->Render();

		break;
	case TestMapEditor::Mode::GAME:
		gameScene->Render();
		break;
	}
}

void TestMapEditor::PostRender()
{
}

void TestMapEditor::ResizeScreen()
{
}