#pragma once
#include "Execute.h"

class TestBlur : public Execute
{
public:
	TestBlur();
	~TestBlur();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	void BuildGeometry();

private:
	class CubeSky* sky;

	wstring floor;
	wstring stone;
	wstring brick;

	GameModel* envMappedSphere;
	GameModel* bunny;
	D3DXVECTOR3 rotation;

	Material* sphereMaterial;
	MeshSphere* sphere[10];

	Material* cylinderMaterial;
	MeshCylinder* cylinder[10];

	Material* boxMaterial;
	MeshCube* box;

	Material* planeMaterial;
	MeshGrid* plane;

	Viewport* viewport;
	DepthStencilView* dsv;
	RenderTargetView* rtv;

	Render2D * render2D;

	class Blur * blur;
};


