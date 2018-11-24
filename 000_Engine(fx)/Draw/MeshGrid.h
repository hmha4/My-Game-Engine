#pragma once
#include "Mesh.h"

class MeshGrid : public Mesh
{
public:
	MeshGrid(Material* material, UINT countX, UINT countZ, float sizeX, float sizeZ);
	~MeshGrid();

	void CreateData() override;

private:
	UINT countX, countZ;
	float sizeX, sizeZ;
};