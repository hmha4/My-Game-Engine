#include "stdafx.h"
#include "TestModelAnim.h"

TestModelAnim::TestModelAnim()
{
	kachujin = new GameAnimModel
	{
		Models + L"Kachujin/", L"Kachujin.material",
		Models + L"Kachujin/", L"Kachujin.mesh"
	};
	kachujin->SetShader(Effects + L"003_Model.fx");
	kachujin->SetDiffuse(1, 1, 1, 1);
	kachujin->Scale(0.25f, 0.25f, 0.25f);

	kachujin->AddClip(Models + L"Kachujin/Running.anim");
	kachujin->Play(0, true, 0.0f, 10.0f);
}

TestModelAnim::~TestModelAnim()
{
	SAFE_DELETE(kachujin);
}

void TestModelAnim::Update()
{
	kachujin->Update();
}

void TestModelAnim::PreRender()
{
}

void TestModelAnim::Render()
{
	kachujin->Render();
}

void TestModelAnim::PostRender()
{

}

void TestModelAnim::ResizeScreen()
{
}