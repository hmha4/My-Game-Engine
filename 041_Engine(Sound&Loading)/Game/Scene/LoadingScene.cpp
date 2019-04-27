#include "stdafx.h"
#include "LoadingScene.h"

void LoadingScene::Initialize()
{
	InitUI();
}

void LoadingScene::Ready()
{
	SetUI(true);
}

void LoadingScene::Destroy()
{
	SetUI(false);
}

void LoadingScene::Update()
{
	float val = ProgressBar::Get()->Progress();
	val += 0.2f * Time::Delta();
	ProgressBar::Get()->Progress(val);
	UIManager::Get()->GetRender2D(L"00Loading")->SetUV(L"x", val, 1.0f);
}

void LoadingScene::PreRender()
{
	
}

void LoadingScene::Render()
{
	
}

void LoadingScene::PostRender()
{
}

void LoadingScene::ResizeScreen()
{
}

void LoadingScene::SetUI(bool val)
{
	UIManager::Get()->SetUiRender(L"00Loading", val);
}

void LoadingScene::InitUI()
{
	D3DDesc d3dDesc;
	D3D::GetDesc(&d3dDesc);

	{
		UIManager::UIdesc desc;
		desc.dcImg = new Render2D(Effects + L"041_LoadingScene.fx", true);
		desc.dcImg->Scale(d3dDesc.Width, d3dDesc.Height);
		desc.dcImg->Position(0, 0);
		desc.isRender = true;
		desc.uiImg = new Texture(Textures + L"black.png");
		desc.uiImg2 = new Texture(Textures + L"black.png");
		UIManager::Get()->InputUI(L"00Loading", desc);
	}
}