#include "stdafx.h"
#include "TestModelTool.h"
#include "ModelTool/ModelTool.h"

void TestModelTool::Initialize()
{
	tool = new ModelTool();
	tool->Initialize();
}

void TestModelTool::Ready()
{
	tool->Ready();
}

void TestModelTool::Destroy()
{
	SAFE_DELETE(tool);
}

void TestModelTool::Update()
{
	tool->Update();
}

void TestModelTool::PreRender()
{
	tool->PreRender();
}

void TestModelTool::Render()
{
	tool->Render();
}

void TestModelTool::PostRender()
{
}

void TestModelTool::ResizeScreen()
{
}