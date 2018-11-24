#include "stdafx.h"
#include "TestModel.h"

TestModel::TestModel()
{
	tank = new GameModel
	{
		Models + L"Tank/", L"Tank.material",
		Models + L"Tank/", L"Tank.mesh"
	};

	tank->SetShader(Effects + L"003_Model.fx");
}

TestModel::~TestModel()
{
	SAFE_DELETE(tank);
}

void TestModel::Update()
{
	
}

void TestModel::PreRender()
{
}

void TestModel::Render()
{
	tank->Render();
}

void TestModel::PostRender()
{

}

void TestModel::ResizeScreen()
{
}