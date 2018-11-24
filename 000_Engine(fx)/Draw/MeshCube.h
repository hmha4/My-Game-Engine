#pragma once
#include "Mesh.h"

class MeshCube : public Mesh
{
public:
	MeshCube(Material* material, float width, float height, float depth);
	~MeshCube();

	void CreateData() override;

private:
	float width;
	float height;
	float depth;
};