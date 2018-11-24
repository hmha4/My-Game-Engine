#include "stdafx.h"
#include "MeshSphere.h"

MeshSphere::MeshSphere(Material* material, float radius, UINT stackCount, UINT sliceCount)
	: Mesh(material), radius(radius), stackCount(stackCount), sliceCount(sliceCount)
{

}

MeshSphere::~MeshSphere()
{

}

void MeshSphere::CreateData()
{
	vector<VertexTextureNormalTangent> vertices;
	vertices.push_back(VertexTextureNormalTangent(0, radius, 0, 0, 0, 0, 1, 0, 1, 0, 0));

	float phiStep = Math::PI / stackCount;
	float thetaStep = 2.0f * Math::PI / sliceCount;

	for (UINT i = 1; i <= stackCount - 1; i++)
	{
		float phi = i * phiStep;

		for (UINT j = 0; j <= sliceCount; j++)
		{
			float theta = j * thetaStep;

			D3DXVECTOR3 p = D3DXVECTOR3
			(
				(radius * sinf(phi) * cosf(theta)),
				(radius * cosf(phi)),
				(radius * sinf(phi) * sinf(theta))
			);

			D3DXVECTOR3 t = D3DXVECTOR3
			(
				-radius * sinf(phi) * sinf(theta),
				0,
				radius * sinf(phi) * cosf(theta)
			);

			D3DXVec3Normalize(&t, &t);

			D3DXVECTOR3 n;
			D3DXVec3Normalize(&n, &p);

			D3DXVECTOR2 uv = D3DXVECTOR2(theta / (Math::PI * 2), phi / Math::PI);
			vertices.push_back(VertexTextureNormalTangent(p.x, p.y, p.z, uv.x, uv.y, n.x, n.y, n.z, t.x, t.y, t.z));
		}
	}
	vertices.push_back(VertexTextureNormalTangent(0, -radius, 0, 0, 1, 0, -1, 0, 1, 0, 0));

	this->vertices = new VertexTextureNormalTangent[vertices.size()];
	vertexCount = vertices.size();
	copy(vertices.begin(), vertices.end(), stdext::checked_array_iterator<VertexTextureNormalTangent *>(this->vertices, vertexCount));



	vector<UINT> indices;
	for (UINT i = 1; i <= sliceCount; i++)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	UINT baseIndex = 1;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT i = 0; i < stackCount - 2; i++)
	{
		for (UINT j = 0; j < sliceCount; j++)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	UINT southPoleIndex = vertices.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;

	for (UINT i = 0; i < sliceCount; i++)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	this->indices = new UINT[indices.size()];
	indexCount = indices.size();
	copy(indices.begin(), indices.end(), stdext::checked_array_iterator<UINT *>(this->indices, indexCount));
}
