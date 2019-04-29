#include "Framework.h"
#include "UIManager.h"


UIManager*UIManager::instance = NULL;

void UIManager::Create()
{
	if (instance == NULL)
	{
		instance = new UIManager();
		instance->Ready();
	}
}

void UIManager::Delete()
{
	if (instance != NULL)
	{
		uIter uiter = instance->uiMap.begin();

		for (; uiter != instance->uiMap.end(); uiter++)
		{
			SAFE_DELETE(uiter->second.uiImg);
			SAFE_DELETE(uiter->second.uiImg2);
			SAFE_DELETE(uiter->second.dcImg);
		}
	}
	instance->uiMap.clear();
	SAFE_DELETE(instance->dcblack);
	SAFE_DELETE(instance->blackImg);
	SAFE_DELETE(instance);

}

void UIManager::InputUI(wstring name, UIdesc udesc)
{
	UIdesc uidesc;

	uidesc.dcImg = udesc.dcImg;
	uidesc.uiImg = udesc.uiImg;
	uidesc.dcImg->SRV(uidesc.uiImg->SRV());
	uidesc.uiImg2 = udesc.uiImg2;
	if (uidesc.uiImg2 != NULL)
		uidesc.dcImg->SRVSnd(uidesc.uiImg2->SRV());

	uidesc.isRender = false;

	uiMap[name] = uidesc;
}

void UIManager::SetUiRender(wstring name, bool isRender)
{
	uIter uiter = uiMap.find(name);

	if(uiter!=uiMap.end())
		uiMap[name].isRender = isRender;
}

void UIManager::Ready()
{
	alpha = 0;
	useFade = false;
	float width = Context::Get()->GetViewport()->GetWidth();
	float height = Context::Get()->GetViewport()->GetHeight();
	
	dcblack = new Render2D(L"039_Fade.fx");
	dcblack->Position(0, 0);
	dcblack->Scale(width, height);

	blackImg = new Texture(Textures + L"black.png");
	dcblack->SRV(blackImg->SRV());

	dcblack->GetEffect()->AsScalar("AlphaValue")->SetFloat(alpha);
}

void UIManager::Update()
{
	uIter uiter = uiMap.begin();

	for (; uiter != uiMap.end(); uiter++)
	{
		if (!uiter->second.isRender)continue;
		uiter->second.dcImg->Update();
	}

	if (useFade)
	{
		if (Time::Get()->FPS() >= 60)
		{
			float delta = Time::Delta();
			alpha += timeValue * 0.1f * delta;

			if (alpha > 1)
				endFadeOut = true;
			if (alpha < 0)
				endFadeIn = true;
		}
	}
}

void UIManager::Render()
{
	uIter uiter = uiMap.begin();

	for (; uiter != uiMap.end(); uiter++)
	{
		if (!uiter->second.isRender)continue;

		uiter->second.dcImg->Render();
	}

	if (useFade)
	{
		dcblack->GetEffect()->AsScalar("AlphaValue")->SetFloat(alpha);
		dcblack->Render();
	}
}

void UIManager::Empty()
{
	if (instance != NULL)
	{
		uIter uiter = instance->uiMap.begin();

		for (; uiter != instance->uiMap.end(); uiter++)
		{
			SAFE_DELETE(uiter->second.uiImg);
			SAFE_DELETE(uiter->second.uiImg2);
			SAFE_DELETE(uiter->second.dcImg);
		}
	}
	instance->uiMap.clear();
}

void UIManager::UseFadeOut(float timeVal)
{
	alpha = 0;
	useFade = true;
	timeValue = timeVal;
	endFadeOut = false;
}

void UIManager::UseFadeIn(float timeVal)
{
	alpha = 1;
	useFade = true;
	timeValue = timeVal;
	endFadeIn = false;
}

void UIManager::ResetFadeOut()
{
	alpha = 0;
	useFade = false;
}
