#pragma once
#include "Execute.h"

class TestModelAnim : public Execute
{
public:
	TestModelAnim();
	~TestModelAnim();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	GameAnimModel * kachujin;
};


