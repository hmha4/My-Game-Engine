#pragma once
#include "Execute.h"

class TestGrass : public Execute
{
public:
	TestGrass();
	~TestGrass();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	class Grass * grass;
	class Rain * rain;

	class Terrain * terrain;
};


