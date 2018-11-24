#pragma once
#include "Execute.h"

//	���� : nvidia rain with geometry and billboard and instancing
//	���� : grass with geometry and billboard and instancing(wind, flower(random))
//	TODO : 053_Instancing.hlsl

class TestInstancing : public Execute
{
public:
	TestInstancing();
	~TestInstancing();

	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void ResizeScreen();

private:
	class Terrain * terrain;

	Shader * shader;
	ID3D11Buffer * vertexBuffer[2];
	TextureArray * textureArray;

private:
	struct VertexWorld
	{
		D3DXMATRIX	World;
		UINT		Texture;
	};
};


