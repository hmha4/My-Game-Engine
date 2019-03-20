#include "stdafx.h"
#include "TestMapEditor.h"
#include "Editors/EditorManager.h"

void TestMapEditor::Initialize()
{
	CollisionContext::Get()->AddLayer(L"Collision Weapon");
	CollisionContext::Get()->AddLayer(L"Collision Item");
	CollisionContext::Get()->AddLayer(L"Collision Player");
	CollisionContext::Get()->AddLayer(L"Collision Enemy");
	CollisionContext::Get()->AddLayer(L"Collision Character");
	CollisionContext::Get()->AddLayer(L"Collision World");

	editorMgr = new EditorManager();
	editorMgr->Initialize();
}

void TestMapEditor::Ready()
{
	editorMgr->Ready();
}

void TestMapEditor::Destroy()
{
	SAFE_DELETE(editorMgr);
}

void TestMapEditor::Update()
{
	editorMgr->Update();
}

void TestMapEditor::PreRender()
{
	editorMgr->PreRender();
}

void TestMapEditor::Render()
{
	editorMgr->Render();
}

void TestMapEditor::PostRender()
{
}

void TestMapEditor::ResizeScreen()
{
}