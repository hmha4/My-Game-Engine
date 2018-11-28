#pragma once
#include "Execute.h"

//	Sampling 은 속도 저하를 일으킴
//	-> 그래서 Sampler 대신 Gather라는 걸 사용??

class TestRender2D : public Execute
{
public:
	TestRender2D();
	~TestRender2D();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	class CubeSky* sky;

	wstring floor;
	wstring stone;
	wstring brick;

	GameModel* bunny;

	Material* sphereMaterial;
	MeshSphere* sphere[10];

	Material* cylinderMaterial;
	MeshCylinder* cylinder[10];

	Material* boxMaterial;
	MeshCube* box;

	Material* planeMaterial;
	MeshGrid* plane;

	Material* quadMaterial;
	MeshQuad* quad;

	Viewport* viewport;
	RenderTargetView* rtv;
	DepthStencilView* dsv;

	Render2D * render2D;
};


