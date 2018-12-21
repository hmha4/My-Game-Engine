#pragma once
#include "Execute.h"

class TessTerrain : public Execute
{
public:
	TessTerrain();
	~TessTerrain();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	class CubeSky * sky;
	class Terrain * terrain;
};