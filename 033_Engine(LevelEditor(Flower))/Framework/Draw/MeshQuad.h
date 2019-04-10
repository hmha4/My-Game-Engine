#pragma once
#include "Mesh.h"

class MeshQuad : public Mesh
{
public:
	MeshQuad(Material* material, float width = 1.0f, float height = 1.0f);
	~MeshQuad();

	void CreateData() override;

private:
	float width;
	float height;
};