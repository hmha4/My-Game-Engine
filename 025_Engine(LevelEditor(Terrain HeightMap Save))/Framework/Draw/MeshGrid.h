#pragma once
#include "Mesh.h"

class MeshGrid : public Mesh
{
public:
	MeshGrid(Material* material, UINT countX, UINT countZ, float sizeX, float sizeZ, float offsetU = 1.0f, float offsetV = 1.0f);
	~MeshGrid();

	void CreateData() override;

private:
	UINT countX, countZ;
	float sizeX, sizeZ;
	float offsetU, offsetV;
};