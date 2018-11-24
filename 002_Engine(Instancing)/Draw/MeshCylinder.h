#pragma once
#include "Mesh.h"

class MeshCylinder : public Mesh
{
public:
	MeshCylinder(Material* material, float topRadius, float bottomRadius, float height, UINT sliceCount = 10, UINT stackCount = 10);
	~MeshCylinder();

	void CreateData() override;

private:
	void BuildTopCap(vector<VertexTextureNormalTangent>& vertices, vector<UINT>& indices);
	void BuildBottomCap(vector<VertexTextureNormalTangent>& vertices, vector<UINT>& indices);

private:
	float topRadius;
	float bottomRadius;
	float height;

	UINT sliceCount;
	UINT stackCount;
};