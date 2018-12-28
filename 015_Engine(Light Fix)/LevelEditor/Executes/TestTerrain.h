#pragma once
#include "Systems/IExecute.h"

class TestTerrain : public IExecute
{
public:
	void Initialize() override;
	void Ready() override;
	void Destroy() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;
	void ResizeScreen() override;

private:
	class CubeSky * sky;

	Material * terrainMaterial;
	class Terrain * terrain;

	Material * material;
	class MeshSphere * obj;

	D3DXMATRIX temp;
	float tempX;
};