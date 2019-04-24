#pragma once

class UIManager
{
public:
	static void Create();
	static void Delete();

	static UIManager* Get() { return instance; }
private:
	static UIManager*instance;

public:
	struct UIdesc
	{
		class Render2D*dcImg;
		class Texture*uiImg;
		class Texture*uiImg2 = NULL;
		bool isRender;
	};
public:
	void InputUI(wstring name, UIdesc udesc);
	void SetUiRender(wstring name, bool isRender);
	bool GetUiRenderState(wstring name) { return uiMap[name].isRender; }

	class Render2D*GetRender2D(wstring name) { return uiMap[name].dcImg; }
	void Ready();
	void Update();
	void Render();

	void UseFadeOut(float timeVal);
	void UseFadeIn(float timeVal);
	void ResetFadeOut();
	bool EndFadeOut() { return endFadeOut; }
private:
	typedef map<wstring, UIdesc>::iterator uIter;
private:
	map<wstring, UIdesc> uiMap;

	class Render2D*dcblack;
	class Texture*blackImg;
	float alpha;
	float timeValue;
	bool useFade;
	bool endFadeOut;
	bool endFadeIn;

};