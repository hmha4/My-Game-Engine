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
	class ScatterSky * sky;

	Effect * terrainEffect;
	Material * terrainMaterial;
	class Terrain * terrain;

	Effect * effect;
	Material * material;
	class MeshSphere * obj;
	class MeshCube * obj1;

	class Shadow * shadow;
	class Grass * grass;
};